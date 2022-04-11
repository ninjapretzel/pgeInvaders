#define OLC_PGE_APPLICATION
#include "olcPixelGameEngine.h"
#include <iostream>
#include <vector>
#include <random>
#include <string>
#include <cstdint>

using namespace std;
using namespace olc;

class Invader {

};

double next() {
	return ((double)rand()) / RAND_MAX;
}
int nextInt(int a, int b = 0) {
	double f = next();
	return (int) (a + (b-a)*f);
}

int nextInt() {
	int numBits = 8 * sizeof(int);
	int bits = 0;
	for (int i = 0; i < numBits; i++) {
		if (next() < .5) { continue; }
		int k = 1 << i;
		bits |= k;
	}
	return bits;
}

float nextFloat(float a, float b = 0) {
	double f = next();
	return (float) (a + (b-a)*f);
}
double nextDouble(double a, double b = 0) {
	double f = next();
	return (a + (b-a)*f);
}

struct IntSetting {
	int32_t min, max;
	int32_t next() { return nextInt(min, max+1); }
};

struct vf4d {
	float x,y,z,w;
};

inline vf4d operator +(const vf4d lhs, const vf4d rhs) { return vf4d { lhs.x + rhs.x, lhs.y + rhs.y, lhs.z + rhs.z, lhs.w + rhs.w }; }
inline vf4d operator -(const vf4d lhs, const vf4d rhs) { return vf4d { lhs.x - rhs.x, lhs.y - rhs.y, lhs.z - rhs.z, lhs.w - rhs.w }; }
inline ostream& operator <<(ostream& lhs, const vf4d rhs) {
	lhs << "vf4d { " << rhs.x << ", " << rhs.y << ", " << rhs.z << ", " << rhs.w << " }";
	return lhs;
}


struct HsvSetting {
	float hue;
	float minSat, maxSat;
	float minVal, maxVal;
	vf4d next() {
		return vf4d { 
			nextFloat(-hue, hue), 
			nextFloat(minSat, maxSat), 
			nextFloat(minVal, maxVal), 
			0 
		};
	}
};

int countBits(int64_t data) {
	int cnt = 0;
	while (data != 0) {
		data = data & (data - 1LL);
		cnt++;
	}
	return cnt;
}

int64_t pickBits(int64_t mask, int numBits) {
	int64_t bits = 0;
	for (int i = 0; i < numBits; i++) {
		int pos = nextInt(0, 63);
		int64_t next = 1LL << pos;
		// Not in mask, skip and retry
		bool inMask = (next & mask) != 0;
		if (!inMask) { i--; continue; }
		// Already in bitset, just skip
		bool inBits = (next & bits) != 0;
		if (inBits) { continue; }
		bits |= next;
	}
	return bits;
}

int64_t splatBits(int64_t mask, int minBits) {
	int64_t bits = 0;
	if (minBits >= countBits(mask)) {
		bits |= nextInt();
		bits |= ((int64_t)nextInt()) << 32;
		bits &= mask;
		return bits;
	}
	while (countBits(bits) < minBits) {
		bits = 0;
		bits |= nextInt();
		bits |= ((int64_t)nextInt()) << 32;
		bits &= mask;
	}

	return bits;
}

void renderInvaderSprite(Sprite spr, int64_t bits, Pixel color) {
	int w = spr.width;
	int h = spr.height;
	bool oddW = w % 2 == 1;
	int wbits = (oddW ? 1 : 0) + w / 2;
	for (int yy = 0; yy < h; yy++) {
		for (int xx = 0; xx < wbits; xx++) {
			int i = yy * wbits + xx;
			if (i >= 64) { return; }

			int64_t mask = 1LL << i;
			bool bit = (bits & mask) != 0;

			if (bit) {
				spr.SetPixel(xx, yy, color);
				spr.SetPixel(w - 1 - xx, yy, color);
			}
		}
	}
}

class InvaderSettings {
public:
	IntSetting frames = { 2, 5 };
	IntSetting layers = { 2, 5 };
	IntSetting size = { 2, 5 };
	IntSetting deco = { 2, 5 };
	IntSetting anim = { 2, 5 };
	HsvSetting hsv = { .2f, -.2f, .2f, -.2f, .2f };
	void copy(InvaderSettings other) {
		this->frames = other.frames;
		this->layers = other.layers;
		this->size = other.size;
		this->deco = other.deco;
		this->anim = other.anim;
		this->hsv = other.hsv;
	}
};
InvaderSettings DEFAULT_INVADER_SETTINGS;


vector<Sprite> renderInvaderPoses(int seed, InvaderSettings& sets = DEFAULT_INVADER_SETTINGS) {
	srand(seed);

	int numFrames = sets.frames.next();
	int numLayers = sets.layers.next();
	int width = sets.size.next();
	int height = sets.size.next();
	vector<Sprite> poses;
	vector<int64_t> baseFrame;
	vector<Pixel> colors;
	int fill = (int) sqrt(width*height);
	int maxBits = height * width - (height* (width / 2));
	int64_t bitMask = (1LL << (1 + maxBits)) - 1LL;	


	return poses;
}

class Example : public PixelGameEngine {
public:
	Example() {
		sAppName = "Example";
	}
	int cnt = 0;
	vi2d d = { 0, 2 };
	bool OnUserCreate() override {
		return true;
	}

	bool OnUserUpdate(float fElapsedTime) override {
		// called once per frame
		for (int x = 0; x < ScreenWidth(); x++){
			for (int y = 0; y < ScreenHeight(); y++) {
				//Draw(x, y, Pixel(rand() % 255, rand() % 255, rand() % 255));
			}
		}

//		cout << "tick " << cnt << " randmax " << RAND_MAX << endl;
		cnt++;
		return true;
	}
};


int main() {
	Example demo;
	if (demo.Construct(256, 240, 4, 4)) {
		demo.Start();
	}

	return 0;
}
