#ifndef DEMOAPP_H_
#define DEMOAPP_H_

class DemoApp
{
public:

	DemoApp();
	~DemoApp();

	bool init(const char* fileName);

	bool isRunning() {return m_running;} 
	
	void keyHandler();
	void render();

	void mouseMoved(float x, float y);
	void mouseClick(int button, int action);

	void keyPress( unsigned int param, bool pressed );
	void resize(int width, int height);

	void release();

private:
	bool m_running;
	char m_keys[326];
	float m_camRX, m_camRY;
	int	  m_camID;
	int  m_mousedown;
	float m_lastx, m_lasty;
};
#endif