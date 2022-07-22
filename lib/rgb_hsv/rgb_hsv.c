#include "rgb_hsv.h"

struct rgb_value hsv2rgb(const struct hsv_value val)
{
	struct rgb_value out;
	uint8_t i;
	float C; float ff;
	float r; float g; float b;
	

	if (val.h < 0.0 || 360.0 < val.h ||
		val.s < 0.0 || 1.0 < val.s ||
		val.v < 0.0 || 1.0 < val.v) {
		out.r = 0;
		out.g = 0;
		out.b = 0;
		return out;
	}

	C = val.v * val.s;

	i = (uint8_t)(val.h / 60.0);
	ff = (val.h / 60.0) - (float)i;

	r = val.v;
	g = val.v;
	b = val.v;

	switch (i) {
	case 0:
		g -= C * (1.0 - ff);
		b -= C;
		break;

	case 1:
		r -= C * ff;
		b -= C;
		break;

	case 2:
		r -= C;
		b -= C * (1.0 - ff);
		break;

	case 3:
		r -= C;
		g -= C * ff;
		break;

	case 4:
		r -= C * (1.0 - ff);
		g -= C;
		break;

	case 5:
	default:
		g -= C;
		b -= C * ff;
		break;
	}

	out.r = (uint8_t)(r * 255.0);
	out.g = (uint8_t)(g * 255.0);
	out.b = (uint8_t)(b * 255.0);

	return out;
}

struct hsv_value rgb2hsv(const struct rgb_value val)
{
	struct hsv_value out;

	float r; float g; float b;
	float cmax; float cmin; float diff;

	r = val.r/255.0;
	g = val.g/255.0;
	b = val.b/255.0;

	if (g > b) {
		cmax = (r > g) ? r : g;
		cmin = (r < b) ? r : b;
	} else {
		cmax = (r > b) ? r : b;
		cmin = (r < g) ? r : g;
	}
	diff = cmax - cmin;

	if (diff == 0) {
		out.h = 0;
	} else if (cmax == r) {
		out.h = 60 * ((g - b) / diff);
		if (out.h < 0) {
			out.h = out.h + 360;
		}
	} else if (cmax == g) {
		out.h = 60 * ((b - r) / diff) + 120.0;
	} else if (cmax == b) {
		out.h = 60 * ((r - g) / diff) + 240.0;
	}

	if (cmax == 0) {
		out.s = 0;
	} else {
		out.s = (diff / cmax) * 100;
	}

	out.v = cmax * 100;

	return out;
}
