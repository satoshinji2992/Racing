#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>

using namespace sf;

const int WinWidth = 1024;
const int WinHeight = 768;
const int roadWidth = 2000;
const int segLength = 180;
const int roadCount = 1884;

float angle = 0;
bool turnl = false, turnr = false;

struct point {
	float x, y, z;
	float X, Y;
	float scale, tx, tz;
	point(float _x, float _y, float _z) : x(_x), y(_y), z(_z) {}
	void project(int camX, int camY, int camZ, float angle) {
		tx = (x - camX) * cos(angle) + (z - camZ) * sin(angle);
		tz = -(x - camX) * sin(angle) + (z - camZ) * cos(angle);
		if (tz < 0.1f) tz = 0.1f;
		scale = 1.0f / tz;
		X = (1 + scale * tx) * WinWidth / 2;
		Y = (1 - scale * (y - camY)) * WinHeight / 2;
	}
};

struct nailong
{
	point p[4];
	nailong(float _x, float _y, float _z) : p{
		{_x,_y, _z},{_x + 1200,_y,_z},{_x + 1200,_y + 1800,_z},{_x,_y + 1800,_z}} {}
	bool eat = false;
};

//把一张长方形图片缩放后绘制在四个点上,用ploygon绘制
void DrawNailong(RenderWindow& window, Texture& t, nailong n) {
	ConvexShape polygon(4);
	polygon.setTexture(&t);
	polygon.setPoint(0, Vector2f(n.p[0].X, n.p[0].Y));
	polygon.setPoint(1, Vector2f(n.p[1].X, n.p[1].Y));
	polygon.setPoint(2, Vector2f(n.p[2].X, n.p[2].Y));
	polygon.setPoint(3, Vector2f(n.p[3].X, n.p[3].Y));
	window.draw(polygon);
}

//道路结构体
struct Road
{
	float x, y, z;
	float X, Y, W;
	float scale;
	float tz, tx;

	Road(float _x, int _y, int _z) : x(_x), y(_y), z(_z) {}
	void project(int camX, int camY, int camZ, float angle) {
		tx = (x-camX) * cos(angle) + (z-camZ) * sin(angle);
		tz = -(x-camX) * sin(angle) + (z-camZ) * cos(angle);
		if (tz < 0.1f) tz = 0.1f;
		scale = 1.0f / tz;
		X = (1 + scale * tx) * WinWidth / 2;
		Y = (1 - scale * (y - camY)) * WinHeight / 2;
		W = scale * roadWidth * WinWidth / 2;
	}

};

//绘制梯形
void DrawTrape(RenderWindow& window, Color c, int x1, int y1, int w1, int x2, int y2, int w2,float angle) {
	ConvexShape polygon(4);
	polygon.setFillColor(c);
	polygon.setPoint(0, Vector2f(x1 - w1*cos(angle), y1-w1*sin(angle)));
	polygon.setPoint(1, Vector2f(x2 - w2 * cos(angle), y2 - w2 * sin(angle)));
	polygon.setPoint(2, Vector2f(x2 + w2 * cos(angle), y2 + w2 * sin(angle)));
	polygon.setPoint(3, Vector2f(x1 + w1 * cos(angle), y1+w1 * sin(angle)));
	window.draw(polygon);

}

void DrawEnergy(RenderWindow& window,int num) {
	//从左到右绘制num个能量条
	for (int i = 0; i < num; i++) {
		RectangleShape energy(Vector2f(20, 50));
		energy.setFillColor(Color::Blue);
		energy.setPosition(670 + 30 * i, 700);
		window.draw(energy);
	}
}	



int main()
{
	RenderWindow window(VideoMode(WinWidth, WinHeight), "Racing");
	window.setFramerateLimit(60);

	Texture bg1, bg2, nailong, nailong_head;
	bg1.loadFromFile("cloud.png");
	bg2.loadFromFile("car.png");
	nailong.loadFromFile("nailong.png");
	nailong_head.loadFromFile("nailong_head.png");
	Sprite s(bg1, IntRect(0, 0, WinWidth, WinHeight));
	Sprite c(bg2, IntRect(0, 0, WinWidth, WinHeight));
	Sprite n(nailong, IntRect(0, 0, 533, 500));
	Sprite n_head(nailong_head, IntRect(0, 0, WinWidth, WinHeight));

	SoundBuffer buffer[2];
	Sound sound;
	buffer[0].loadFromFile("qidong.mp3");
	buffer[1].loadFromFile("chongci.mp3");



	//生成道路
	std::vector<Road> roads;
	float x = 0;
	for (int i = 0; i < roadCount; i++)
	{
		//共1884,924-,924+
		float curve;
		if (i <= 123 || (456 < i && i <= 789) || (1234 < i && i <= 1721)) curve = 0.5;
		else curve = -0.5;
		x += curve;
		int y = 0;
		if (i > 300 && i < 1240)
			y = 1600 * sin(i / 30.0 - 10);
		Road r(x * (45), y, (1 + i) * segLength);
		roads.push_back(r);
	}

	//生成nailong
#include <ctime>

	srand(static_cast<unsigned int>(time(0)));

	int position1 = rand() % 500;
	int position2 = rand() % 500 + 500;
	int position3 = rand() % 500 + 1000;
	struct nailong n1(roads[position1].x - roadWidth / 2 + rand() % 1500, roads[position1].y, roads[position1].z);
	struct nailong n2(roads[position2].x - roadWidth / 2 + rand() % 1500, roads[position2].y, roads[position2].z);
	struct nailong n3(roads[position3].x - roadWidth / 2 + rand() % 1500, roads[position3].y, roads[position3].z);




	int camZ = 0;
	int camX = 0;
	int camY =2000 ;
	float distance = 0;
	bool isOut = false;
	bool isFly = false;
	int energy = 700;
	int num;
	int round = 0;
	int hit = 0;
	int first = 0;
	int score = 0;
	int flyTime=0;


	//在画面上显示距离
	Font font;
	font.loadFromFile("arial.ttf");
	Text text;
	text.setFont(font);
	text.setCharacterSize(36);
	text.setFillColor(Color::Red);
	text.setPosition(660, 600);


	Clock clock;
	int counter = 0;
	bool start = false;
	Text timerText;
	timerText.setFont(font);
	timerText.setString("Press Enter to start");
	timerText.setCharacterSize(48);
	timerText.setFillColor(Color::Cyan);
	timerText.setPosition(0, 0);

	Text startText;
	startText.setFont(font);
	startText.setString("Press WS to move\nPress AD to turn\nPress F to fly(cost 1000 energy)\nPress Space to accelerate");
	startText.setCharacterSize(60);
	startText.setFillColor(Color::Black);
	startText.setPosition(120, 100);

	//检测键盘输入
	while (window.isOpen())
	{
		Event event;
		while (window.pollEvent(event))
		{
			if (event.type == Event::Closed) window.close();
		}

		//按下enter开始

		if (Keyboard::isKeyPressed(Keyboard::Enter)) {
			if (start == false) {
				sound.setBuffer(buffer[0]);
				sound.play();
			}
			start = true;	
		}
		if (start) {


			Time time = clock.getElapsedTime();
			if (time.asSeconds() >= 1) {
				counter++;  // 增加计数
				clock.restart();  // 重置时钟
			}
			timerText.setString(std::to_string(counter));

			if (Keyboard::isKeyPressed(Keyboard::A)) {
				angle += 0.009;
				turnl = true;
			}
			else turnl = false;

			if (Keyboard::isKeyPressed(Keyboard::D)) {
				angle -= 0.009;
				turnr = true;
			}
			else turnr = false;

			if (isFly == false) {
				if (isOut == false) {
					if (Keyboard::isKeyPressed(Keyboard::W)) {
						camZ += 3 * segLength * cos(-angle);
						camX += 3 * segLength * sin(-angle);
						distance += 0.6;
					}
					if (Keyboard::isKeyPressed(Keyboard::Space) && Keyboard::isKeyPressed(Keyboard::W) && energy > 0) {
						camZ += 2 * segLength * cos(-angle);
						camX += 2 * segLength * sin(-angle);
						distance += 0.4;
						energy -= 2;
					}
				}
				else {
					if (Keyboard::isKeyPressed(Keyboard::W)) {
						camZ += segLength * cos(-angle);
						camX += segLength * sin(-angle);
						distance += 0.2;
						if (Keyboard::isKeyPressed(Keyboard::Space))
							energy -= 1;
					}
				}

				if (Keyboard::isKeyPressed(Keyboard::S)) {
					camZ -= segLength * cos(angle);
					camX -= segLength * sin(-1 * angle);
					distance -= 0.2;
				}

				if (Keyboard::isKeyPressed(Keyboard::Space) && Keyboard::isKeyPressed(Keyboard::S))
					camZ -= 1 * segLength;

				//如果energy>1000,按下f减去1000energy,并且飞行
				if (Keyboard::isKeyPressed(Keyboard::F) && energy >= 1000) {
					energy -= 1000;
					isFly = true;
					sound.setBuffer(buffer[1]);
					sound.play();
				}
			}
			if (isFly == true) {
				if (Keyboard::isKeyPressed(Keyboard::W)) {
					camZ += 8 * segLength * cos(-angle);
					camX += 8 * segLength * sin(-angle);
					distance += 1.6;
				}

				flyTime++;
				if (flyTime > 300) {
					isFly = false;
					flyTime = 0;
				}
			}
		}
		//当撞到nailong时,能量增加
		if (!n1.eat && ((camZ < n1.p[0].z + 100) && (camZ > n1.p[0].z - 100)) && camX > n1.p[0].x - 120 && camX < n1.p[1].x + 120) {
			n1.eat = true;
			energy += 100;
			hit = 30;
		}
		if (!n2.eat && ((camZ < n2.p[0].z + 100) && (camZ > n2.p[0].z - 100)) && camX > n2.p[0].x - 120 && camX < n2.p[1].x + 120) {
			n2.eat = true;
			energy += 100;
			hit = 30;
		}
		if (!n3.eat && ((camZ < n3.p[0].z + 100) && (camZ > n3.p[0].z - 100)) && camX > n3.p[0].x - 120 && camX < n3.p[1].x + 120) {
			n3.eat = true;
			energy += 100;
			hit = 30;
		}



		int totalLength = roadCount * segLength;
		if (camZ >= totalLength) {
			camZ -= totalLength;
			n1.eat = false;
			n2.eat = false;
			n3.eat = false;
			round++;
			//每一轮重新生成nailong
			position1 = rand() % 500 + 200;
			position2 = rand() % 500 + 700;
			position3 = rand() % 500 + 1200;
			n1 = struct nailong(roads[position1].x - roadWidth / 2 + rand() % 1500, roads[position1].y, roads[position1].z);
			n2 = struct nailong(roads[position2].x - roadWidth / 2 + rand() % 1500, roads[position2].y, roads[position2].z);
			n3 = struct nailong(roads[position3].x - roadWidth / 2 + rand() % 1500, roads[position3].y, roads[position3].z);
		}
		if (camZ < 0) camZ += totalLength;


		window.clear();

		int minY = WinHeight;
		int startPos = camZ / segLength;
		//如果isFly,camY逐步增加
		if (isFly) {
			if (camY < 6000) {
				camY += 80;
			}
		}
		else {
			if (camY >= 2000 + roads[startPos].y) {
				camY -= 100;
			}
			if (camY < 2000 + roads[startPos].y) {
				camY = 2000 + roads[startPos].y;
			}
		}

		// ,否则camY=2000+roads[startPos].y

		if (camX < roads[startPos].x + roadWidth / 1.5 && camX>roads[startPos].x - roadWidth / 1.5) isOut = false;
		else isOut = true;

		window.draw(s);


		//计算nailong的X和Y;
		n1.p[0].project(camX, camY, camZ, angle);
		n1.p[1].project(camX, camY, camZ, angle);
		n1.p[2].project(camX, camY, camZ, angle);
		n1.p[3].project(camX, camY, camZ, angle);

		n2.p[0].project(camX, camY, camZ, angle);
		n2.p[1].project(camX, camY, camZ, angle);
		n2.p[2].project(camX, camY, camZ, angle);
		n2.p[3].project(camX, camY, camZ, angle);

		n3.p[0].project(camX, camY, camZ, angle);
		n3.p[1].project(camX, camY, camZ, angle);
		n3.p[2].project(camX, camY, camZ, angle);
		n3.p[3].project(camX, camY, camZ, angle);


		for (int i = startPos; i < startPos + 300; i++) {
			Road& now = roads[i % roadCount];
			now.project(camX, camY, camZ - (i >= roadCount ? totalLength : 0), angle);
			if (!i) continue;
			if (now.Y < minY) {
				minY = now.Y;
			}
			else {
				continue;
			}
			Road& prev = roads[(i - 1) % roadCount];

			Color grass = i % 2 ? Color(12, 210, 16) : Color(0, 199, 0);
			Color edge = i % 2 ? Color(0, 0, 0) : Color(255, 255, 255);
			Color road = i % 2 ? Color(105, 105, 105) : Color(101, 101, 101);
			//绘制道路
			if (turnl) {
				DrawTrape(window, grass, prev.X, prev.Y, WinWidth * 10, now.X, now.Y, WinWidth * 10, -0.1);
				DrawTrape(window, edge, prev.X, prev.Y, prev.W * 1.3, now.X, now.Y, now.W * 1.3, -0.1);
				DrawTrape(window, road, prev.X, prev.Y, prev.W, now.X, now.Y, now.W, -0.1);
			}
			else if (turnr) {
				DrawTrape(window, grass, prev.X, prev.Y, WinWidth * 10, now.X, now.Y, WinWidth * 10, 0.1);
				DrawTrape(window, edge, prev.X, prev.Y, prev.W * 1.3, now.X, now.Y, now.W * 1.3, 0.1);
				DrawTrape(window, road, prev.X, prev.Y, prev.W, now.X, now.Y, now.W, 0.1);
			}
			else {
				DrawTrape(window, grass, prev.X, prev.Y, WinWidth * 10, now.X, now.Y, WinWidth * 10, 0);
				DrawTrape(window, edge, prev.X, prev.Y, prev.W * 1.3, now.X, now.Y, now.W * 1.3, 0);
				DrawTrape(window, road, prev.X, prev.Y, prev.W, now.X, now.Y, now.W, 0);
			}

		}

		if (angle > 1) angle = 1;
		if (angle < -1) angle = -1;

		//绘制nailong
		DrawNailong(window, nailong, n1);
		DrawNailong(window, nailong, n2);
		DrawNailong(window, nailong, n3);

		if (hit > 0) window.draw(n_head);


		//绘制车
		window.draw(c);

		//打印距离和能量
		text.setString(std::to_string(distance));
		window.draw(text);
		num = (energy + 99) / 100;
		DrawEnergy(window, num);
		//打印时间
		window.draw(timerText);

		//打印round,后面加上"/10"
		Text roundText;
		roundText.setFont(font);
		roundText.setString(std::to_string(round) + "/10");
		roundText.setCharacterSize(36);
		roundText.setFillColor(Color::Red);
		roundText.setPosition(0, 100);
		window.draw(roundText);

		if (!start)
			window.draw(startText);

		hit--;

		//当round=10时,黑屏并显示游戏结束,打印时间
		if (round == 10) {
			window.clear();
			Text endText;
			endText.setFont(font);
			endText.setString("Game Over");
			endText.setCharacterSize(48);
			endText.setFillColor(Color::Red);
			endText.setPosition(400, 300);
			window.draw(endText);
			Text timeText;
			timeText.setFont(font);
			if (first < 5) {
				score = counter;
			}
			timeText.setString("Time: " + std::to_string(score) + "s");
			timeText.setCharacterSize(48);
			timeText.setFillColor(Color::Red);
			timeText.setPosition(400, 400);
			window.draw(timeText);

			first++;
		}

		window.display();
	}

	return 0;
}																