#define OLC_PGE_APPLICATION
#include "olcPixelGameEngine.h"
#include <iostream>
#include <vector>
#include <random>
#include <string>
#include <cstdint>

using namespace std;
using namespace olc;


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

Pixel nextColor() {
	return Pixel(rand()%255, rand()%255, rand()%255);
}

float nextFloat() { return (float) next(); }
float nextFloat(float a, float b = 0) {
	double f = next();
	return (float) (a + (b-a)*f);
}
double nextDouble(double a, double b = 0) {
	double f = next();
	return (a + (b-a)*f);
}


struct vf4d {
	float x,y,z,w;
};
vf4d toHsv(Pixel px) {
	float r = px.r / 255.0f;
	float g = px.g / 255.0f;
	float b = px.b / 255.0f;
	float a = px.a / 255.0f;
	vf4d hsva = {0,0,0,a};

	float mx = max(max(r, g), b);
	if (mx <= 0) { return hsva; }

	hsva.z = mx;
	float mn = min(min(r,g), b);
	float delta = mx - mn;
	hsva.y = delta / mx;

	float h;
	if (r == mx) { 
		h = (g - b) / delta;
	} else if (g == mx) {
		h = 2 + (b - r) / delta;
	} else {
		h = 4 + (r - g) / delta;
	}
	h /= 6.0f;
	if (h < 0) { h += 1.0f; }
	hsva.x = h;
	return hsva;
}
float clamp(float v, float mn = 0, float mx = 1) { return (v < mn) ? mn : ((v > mx) ? mx : v); }

Pixel fromHsv(float h, float s, float v, float a) {
	h = fmod(h, 1.0f);
	s = clamp(s);
	v = clamp(v);
	a = clamp(a);

	int i;
	float f,p,q,t;
	if (s == 0) {
		return Pixel { (uint8_t)(v * 255),(uint8_t)(v * 255),(uint8_t)(v * 255),(uint8_t)(a * 255) };
	}

	h *= 6.0f;
	i = (int)h;
	f = h - i;
	p = v * (1 - s);
	q = v * (1 - s * f);
	t = v * (1 - s * (1 - f) );
	
	float r = 0, g = 0, b = 0;
	if (i == 0) { r = v; g = t; b = p; }
	if (i == 1) { r = q; g = v; b = p; }
	if (i == 2) { r = p; g = v; b = t; }
	if (i == 3) { r = p; g = q; b = v; }
	if (i == 4) { r = t; g = p; b = v; }
	if (i == 5) { r = v; g = p; b = q; }
	return Pixel { (uint8_t)(r * 255),(uint8_t)(g * 255),(uint8_t)(b * 255),(uint8_t)(a * 255) };
}
Pixel fromHsv(vf4d v) { return fromHsv(v.x, v.y, v.z, v.w); }

inline vf4d operator +(const vf4d lhs, const vf4d rhs) { return vf4d { lhs.x + rhs.x, lhs.y + rhs.y, lhs.z + rhs.z, lhs.w + rhs.w }; }
inline vf4d operator -(const vf4d lhs, const vf4d rhs) { return vf4d { lhs.x - rhs.x, lhs.y - rhs.y, lhs.z - rhs.z, lhs.w - rhs.w }; }
inline ostream& operator <<(ostream& lhs, const vf4d rhs) {
	lhs << "vf4d { " << rhs.x << ", " << rhs.y << ", " << rhs.z << ", " << rhs.w << " }";
	return lhs;
}
inline ostream& operator <<(ostream& lhs, const Pixel rhs) {
	lhs << "Pixel { " << (int)rhs.r << ", " << (int)rhs.g << ", " << (int)rhs.b << ", " << (int)rhs.a << " }";
	return lhs;
}
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

void renderInvaderSprite(Sprite* spr, int64_t bits, Pixel color) {
	int w = spr->width;
	int h = spr->height;
	bool oddW = w % 2 == 1;
	int wbits = (oddW ? 1 : 0) + w / 2;
	for (int yy = 0; yy < h; yy++) {
		for (int xx = 0; xx < wbits; xx++) {
			int i = yy * wbits + xx;
			if (i >= 64) { return; }

			int64_t mask = 1LL << i;
			bool bit = (bits & mask) != 0;

			if (bit) {
				spr->SetPixel(xx, yy, color);
				spr->SetPixel(w - 1 - xx, yy, color);
			}
		}
	}
	// cout << "Rendered " << bits << " to sprite with color " << color << endl;
}


struct IntSetting {
	int32_t min, max;
	int32_t next() { return nextInt(min, max + 1); }
};

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

class InvaderSettings {
public:
	IntSetting frames = { 2, 5 };
	IntSetting layers = { 2, 4 };
	IntSetting size = { 4, 8 };
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



vector<Sprite*> renderInvaderPoses(int seed, InvaderSettings& sets = DEFAULT_INVADER_SETTINGS) {
	srand(seed);

	int numFrames = sets.frames.next();
	int numLayers = sets.layers.next();
	int width = sets.size.next();
	int height = sets.size.next();
	vector<Sprite*> poses;
	vector<int64_t> baseFrame;
	vector<Pixel> colors;
	int fill = (int) sqrt(width*height);
	int maxBits = height * width - (height* (width / 2));
	int64_t bitMask = (1LL << (1 + maxBits)) - 1LL;	

	vf4d baseColor = toHsv(nextColor());
	baseFrame.push_back(pickBits(bitMask, fill));
	colors.push_back(fromHsv(baseColor));

	for (int i = 1; i < numLayers; i++) {
		int numDeco = sets.deco.next();
		baseFrame.push_back(pickBits(bitMask, numDeco));
		vf4d colorMod = sets.hsv.next();
		colors.push_back(fromHsv(baseColor + colorMod));
	}

	for (int k = 0; k < numFrames; k++) {
		Sprite* spr = new Sprite(width, height);

		for (int i = 0; i < numLayers; i++) {
			uint64_t bits = baseFrame[i];

			if (k != 0) {
				int numFlips = sets.anim.next();
				uint64_t mask = pickBits(bitMask, numFlips);
				bits ^= mask;
			}
			renderInvaderSprite(spr, bits, colors[i]);
		}

		poses.push_back(spr);
	}

	return poses;
}


class Invader {
public:
	vi2d position;
	int seed;
	int speed;
	int framesPerTick;
	int framesSinceTick;
	int frame;
	vector<Sprite*> poses;

	Invader(int seed, InvaderSettings sets = DEFAULT_INVADER_SETTINGS) {
		this->seed = seed;
		poses = renderInvaderPoses(seed, sets);
		speed = nextInt(1, 4);
		if (next() < .5) { speed *= -1; }
		framesPerTick = nextInt(5, 10);
		framesSinceTick = 0;
		frame = 0;
		position = {0,0};
		// cout << "Set up invader with " << poses.size() << " frames." << endl;
	}

	void Update(PixelGameEngine& g) {
		framesSinceTick++;
		if (framesSinceTick == framesPerTick) {
			framesSinceTick = 0;
			frame = (frame + 1) % poses.size();
			
			position.x += speed;
			if (speed > 0) {
				if (position.x >= g.ScreenWidth() - poses[frame]->width) {
					position.x = g.ScreenWidth() - poses[frame]->width;
					position.y += 1;
					speed *= -1;
				} 

			} else {
				if (position.x <= 0) {
					position.x = 0;
					position.y += 1;
					speed *= -1;
				}
			}
			
			if (position.y >= g.ScreenHeight() / 2) {
				position.y = g.ScreenHeight() / 2;
			}
		}

		g.DrawSprite(position, poses[frame]);
	}
};

class Example : public PixelGameEngine {
public:
	vector<Invader> invaders;
	Example() {
		sAppName = "Example";
		vector<int> seeds;
		int nInvaders = 40;
		for (int i = 0; i < nInvaders; i++) {
			seeds.push_back(rand());
		}
		
		for (int i = 0; i < nInvaders; i++) {
			invaders.push_back(Invader(seeds[i]));
			invaders[i].position.x = nextInt(0, ScreenWidth());
			invaders[i].position.y = nextInt(0, ScreenHeight()/2);
		}


	}
	
	bool OnUserCreate() override {
		return true;
	}

	bool OnUserUpdate(float fElapsedTime) override {
		Clear(Pixel(5, 20, 10));
		
		for (int i = 0; i < invaders.size(); i++) {
			invaders[i].Update(*this);
		}
		return true;
	}
};


int main() {
	Example demo;
	if (demo.Construct(256, 240, 4, 4, false, true)) {
		demo.Start();
	}

	return 0;
}
