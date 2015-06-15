#ifndef BACKGROUND_H
#define BACKGROUND_H

class Background {
	public:
        static int init();

		Background();

		void draw();
	private:
		static const int STARS = 200;
		float starx[STARS];
		float stary[STARS];
		float starz[STARS];

        static GLuint planet;
};

#endif

