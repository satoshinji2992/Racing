#include <SFML/Graphics.hpp>

using namespace sf;

const int WinWidth = 1024;
const int WinHeight = 768;
const int roadWidth = 2000;
const int segLength = 180;
const int roadCount = 1884;

//��·�ṹ��
struct Road
{
	float x, y, z;
	float X, Y, W;
	float scale,angle;

	Road(float _x, int _y, int _z) : x(_x), y(_y), z(_z) {}
	void project(int camX, int camY, int camZ)
	{
		scale = 1.0f / (z - camZ);
		X = (1 + scale * (x - camX)) * WinWidth / 2;
		Y = (1 - scale * (y - camY)) * WinHeight / 2;
		W = scale * roadWidth * WinWidth / 2;
	}

};

//��������
void DrawTrape(RenderWindow& window, Color c, int x1, int y1, int w1, int x2, int y2, int w2) {
	ConvexShape polygon(4);
	polygon.setFillColor(c);
	polygon.setPoint(0, Vector2f(x1 - w1, y1));
	polygon.setPoint(1, Vector2f(x2 - w2, y2));
	polygon.setPoint(2, Vector2f(x2 + w2, y2));
	polygon.setPoint(3, Vector2f(x1 + w1, y1));
	window.draw(polygon);

}

void DrawEnergy(RenderWindow& window,int num) {
	//�����һ���num��������
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

	Texture bg1,bg2;
	bg1.loadFromFile("cloud.png");
	bg2.loadFromFile("car.png");
	Sprite s(bg1, IntRect(0, 0, WinWidth, WinHeight / 2));
	Sprite c(bg2, IntRect(0, 0, WinWidth, WinHeight));
	//���ɵ�·
	std::vector<Road> roads;
	float x = 0;
	for (int i = 0; i < roadCount; i++)
	{
		//��1884,924-,924+
		float curve;
		if(i<123||(456<i&&i<789)||(1234<i&&i<1702)) curve = 0.5;
		else curve = -0.5;
		x += curve;
        Road r(x*(45), 1600 * sin(i / 30.0), (1 + i) * segLength);
		roads.push_back(r);
	}

	int camZ = 0;
	int camX = 0;
	int camY;
	float distance = 0;
	bool isOut = false;
	bool isFly = false;
	int num = 7;
	int energy = 698;

	//�ڻ�������ʾ����
	Font font;
	font.loadFromFile("arial.ttf");
	Text text;
	text.setFont(font);
	text.setCharacterSize(36);
	text.setFillColor(Color::Red);
	text.setPosition(660, 600);

    
	Clock clock;
	bool start = false;
	Text timerText;
	timerText.setFont(font);
	timerText.setString("Press Enter to start");
	timerText.setCharacterSize(48);
	timerText.setFillColor(Color::Cyan);
	timerText.setPosition(0, 0);

	//����������
	while (window.isOpen())
	{
		Event event;
		while (window.pollEvent(event))
		{
			if (event.type == Event::Closed) window.close();
		}	
		
		//����enter��ʼ
		
		if (Keyboard::isKeyPressed(Keyboard::Enter)) start = true;
		if (start) {
			Time time = clock.getElapsedTime();
			int timer = static_cast<int>(time.asSeconds());
			timerText.setString(std::to_string(timer));


			if (isOut == false) {
				if (Keyboard::isKeyPressed(Keyboard::W)) {
					camZ += 3 * segLength;
					distance += 0.6;
				}
				if (Keyboard::isKeyPressed(Keyboard::Space) && Keyboard::isKeyPressed(Keyboard::W)&&energy!=0) {
					camZ += 2 * segLength;
					distance += 0.4;
					energy -= 1;
				}
			}
			else {
				if (Keyboard::isKeyPressed(Keyboard::W)) {
					camZ += segLength;
					distance += 0.2;
				}
			}

			if (Keyboard::isKeyPressed(Keyboard::S)) {
				camZ -= segLength;
				distance -= 0.2;
			}
			if (Keyboard::isKeyPressed(Keyboard::W) || Keyboard::isKeyPressed(Keyboard::S)) {
				if (Keyboard::isKeyPressed(Keyboard::A)) camX -= segLength;
				if (Keyboard::isKeyPressed(Keyboard::D)) camX += segLength;
			}

			if (Keyboard::isKeyPressed(Keyboard::Space) && Keyboard::isKeyPressed(Keyboard::S)) camZ -= 1 * segLength;
		}

		int totalLength = roadCount * segLength;
		if (camZ >= totalLength) camZ -= totalLength;
		if (camZ < 0) camZ += totalLength;


		window.clear();

		int minY = WinHeight;
		int startPos = camZ / segLength;
		camY = 1600 + roads[startPos].y;
			
		if (camX < roads[startPos].x+roadWidth/1.5&&camX>roads[startPos].x - roadWidth / 1.5) isOut = false;
		else isOut = true;

		

		for (int i = startPos; i < startPos + 300;i++) {
			Road& now = roads[i%roadCount];
			now.project(camX, camY, camZ-(i>=roadCount? totalLength : 0 ));
			if (!i) continue;
			if (now.Y < minY) {
				minY = now.Y;
			}
			else {
				continue;
			}
			Road& prev = roads[(i - 1)%roadCount];

			Color grass = i % 2 ? Color(12, 210, 16) : Color(0, 199, 0);
			Color edge = i % 2 ? Color(0, 0,0) : Color(255, 255, 255);
			Color road= i %2 ? Color(105,105,105) : Color(101,101,101);
			DrawTrape(window, grass, prev.X, prev.Y, WinWidth*10, now.X, now.Y, WinWidth*10);
			DrawTrape(window, edge, prev.X, prev.Y, prev.W*1.3, now.X, now.Y, now.W*1.3);
			DrawTrape(window, road, prev.X, prev.Y, prev.W, now.X, now.Y, now.W);
			//���Ƶ�·
			s.setTextureRect(IntRect(0, 0, WinWidth, minY));
			window.draw(s);
		}

		window.draw(c);

		text.setString(std::to_string(distance));
		window.draw(text);
		num = (energy+1) / 100;
		DrawEnergy(window, num);
		window.draw(timerText);
		window.display();
	}

	return 0;
}																