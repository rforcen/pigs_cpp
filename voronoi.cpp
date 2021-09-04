// Voronoi tiles

#include "Thread.h"
#include "Timer.h"
#include <Magick++.h>
#include <climits>

typedef uint32_t u32;

class Point
{
public:
  Point(u32 x, u32 y, u32 color) : x(x), y(y), color(color) {}
  static Point rand(int w, int h) { return Point(::rand() % w, ::rand() % h, 0xff000000 | (::rand() % 0xffffff)); }
  u32 x, y, color;
};

class Voronoi
{
public:
  int w, h;
  vector<Point> points;
  u32 *image = nullptr;

  Voronoi(int w, int h, vector<Point> points) : w(w), h(h), points(points), image(new u32[w * h]) {}

  inline int distSq(int i, int j, Point &p)
  {
    int xd = i - p.x, yd = j - p.y;
    return xd * xd + yd * yd;
  }

  u32 gen_pixel(int index)
  {
    auto i = index % w, j = index / h;

    int dist = INT_MAX;
    u32 color = 0;
    for (auto p : points)
    {
      int d = distSq(i, j, p);
      if (d < 3)
      {
        color = 0;
        break;
      }

      if (d < dist)
      {
        dist = d;
        color = p.color;
      }
    }
    return (color == 0) ? 0xff000000 : color;
  }

  void generate_st()
  {
    for (int index = 0; index < w * h; index++)
      image[index] = gen_pixel(index);
  }

  void generate_mt()
  {
    Thread(w * h).run(
        [this](int index)
        {
          image[index] = gen_pixel(index);
        });
  }

  void
  writepng(const char *name)
  {
    Magick::InitializeMagick("voronoi");
    Magick::Image(w, h, "RGBA", Magick::CharPixel, image).write(name);
  }

  ~Voronoi()
  {
    if (image != nullptr)
      delete[] image;
  }
};

void test_voronoi()
{
  const int n = 2800, n_points = n / 2;
  vector<Point> points;

  // generate random points
  for (int i = 0; i < n_points; i++)
    points.push_back(Point::rand(n, n));

  printf("voronoi %dx%d=%d, points:%d, on %d cpus...\n", n, n, n * n, n_points, Thread::nCpus());

  Voronoi voronoi = Voronoi(n, n, points);
  auto t0 = Timer();

  voronoi.generate_mt();
  printf("mt: %ldms, writing voronoi.png\n", t0.lap());

  voronoi.writepng("voronoi.png");
}

int main()
{
  test_voronoi();

  return 0;
}