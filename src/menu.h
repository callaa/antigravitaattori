#ifndef _MENU_H_
#define _MENU_H_

class Menu {
	public:
		Menu();
		static int init();

		int show();
	private:
		void togglePlayer(int p);
		void drawPlayer(int p);
		void update();

		int width, height;
		float anim[4];
		float startanim;
		bool canstart;
		static const float ANIMLEN;
		static GLuint keys[4];
};

#endif

