// mandel
// g++ -O3 mandel.cpp -o mandel -pthread
// using magick: `Magick++-config --cxxflags --cppflags`

#include "Thread.h"
#include "Timer.h"
#include <complex>
#include <Magick++.h>

using std::complex;

typedef uint32_t u32;
typedef complex<float> Cmplx;

class Mandelbrot
{
private:
  const u32 fire_pallete[265] = {
      0, 0, 4, 12, 16, 24, 32, 36, 44, 48, 56, 64, 68, 76, 80, 88, 96, 100, 108,
      116, 120, 128, 132, 140, 148, 152, 160, 164, 172, 180, 184, 192, 200, 1224,
      3272, 4300, 6348, 7376, 9424, 10448, 12500, 14548, 15576, 17624,
      18648, 20700, 21724, 23776, 25824, 26848, 28900, 29924, 31976, 33000,
      35048, 36076, 38124, 40176, 41200, 43248, 44276, 46324, 47352, 49400,
      51452, 313596, 837884, 1363196, 1887484, 2412796, 2937084, 3461372, 3986684,
      4510972, 5036284, 5560572, 6084860, 6610172, 7134460, 7659772, 8184060,
      8708348, 9233660, 9757948, 10283260, 10807548, 11331836, 11857148, 12381436,
      12906748, 13431036, 13955324, 14480636, 15004924, 15530236, 16054524, 16579836,
      16317692, 16055548, 15793404, 15269116, 15006972, 14744828, 14220540, 13958396,
      13696252, 13171964, 12909820, 12647676, 12123388, 11861244, 11599100, 11074812,
      10812668, 10550524, 10288380, 9764092, 9501948, 9239804, 8715516, 8453372,
      8191228, 7666940, 7404796, 7142652, 6618364, 6356220, 6094076, 5569788,
      5307644, 5045500, 4783356, 4259068, 3996924, 3734780, 3210492, 2948348,
      2686204, 2161916, 1899772, 1637628, 1113340, 851196, 589052, 64764,
      63740, 62716, 61692, 59644, 58620, 57596, 55548, 54524, 53500, 51452,
      50428, 49404, 47356, 46332, 45308, 43260, 42236, 41212, 40188, 38140,
      37116, 36092, 34044, 33020, 31996, 29948, 28924, 27900, 25852, 24828,
      23804, 21756, 20732, 19708, 18684, 16636, 15612, 14588, 12540, 11516,
      10492, 8444, 7420, 6396, 4348, 3324, 2300, 252, 248, 244, 240, 236, 232,
      228, 224, 220, 216, 212, 208, 204, 200, 196, 192, 188, 184, 180, 176, 172,
      168, 164, 160, 156, 152, 148, 144, 140, 136, 132, 128, 124, 120, 116, 112,
      108, 104, 100, 96, 92, 88, 84, 80, 76, 72, 68, 64, 60, 56, 52, 48, 44, 40,
      36, 32, 28, 24, 20, 16, 12, 8, 0, 0};

public:
  int w = 0, h = 0, iters = 200;
  Cmplx center = Cmplx(0.5f, 0.0f), range = Cmplx(-2.0f, 2.0f);
  u32 *image = nullptr;

private:
  inline Cmplx do_scale(Cmplx cr, Cmplx range, float iw, float jh)
  {
    return cr + Cmplx((range.imag() - range.real()) * iw, (range.imag() - range.real()) * jh);
  }

public:
  Mandelbrot(u32 w, u32 h) : w(w), h(h), image(new u32[w * h]) {}

  Mandelbrot(u32 w, u32 h, u32 iters, Cmplx center, Cmplx range) : w(w), h(h), iters(iters), center(center), range(range), image(new u32[w * h])
  {
  }

  ~Mandelbrot()
  {
    if (image != nullptr)
      delete[] image;
  }

  int size_bytes() { return w * h * sizeof(*image); }
  int image_size() { return w * h; }

  u32 gen_pixel(int index, float scale)
  {
    auto cr = Cmplx(range.real(), range.real());

    auto i = index % w, j = index / w;
    const Cmplx c0 = scale * do_scale(cr, range, float(i) / w, float(j) / h) - center;
    Cmplx z = c0;

    int ix = iters;
    for (int it = 0; it < iters; it++)
    {
      z = z * z + c0;
      if (norm(z) > 4.0f)
      {
        ix = it;
        break;
      }
    }
    return 0xff000000 | ((ix == iters)
                             ? 0
                             : fire_pallete[256 * ix / 50]);
  }

  // single thread
  void maneldebrot_st()
  {
    const float scale = 0.8 * w / h;
    auto cr = Cmplx(range.real(), range.real());

    for (auto index = 0; index < image_size(); index++)
      image[index] = gen_pixel(index, scale);
  }

  // multithread version
  void maneldebrot_mt()
  {
    const float scale = 0.8 * w / h;
    Thread(image_size()).run([this, scale](int index)
                             { image[index] = gen_pixel(index, scale); });
  }

  void writepng(const char *name)
  {
    Magick::InitializeMagick("mandel");
    Magick::Image(w, h, "RGBA", Magick::CharPixel, image).write(name);
  }

  void writefile(const char *name) // write raw binary file
  {
    auto fh = fopen(name, "w+b");
    fwrite(image, w * h, sizeof(*image), fh);
    fclose(fh);
  }
};

void test_mandel()
{
  const int n = 1024 * 4, iters = 200;
  auto mandel = Mandelbrot(n, n, iters, Cmplx(0.5f, 0.0f), Cmplx(-2.0f, 2.0f));

  printf("mandelbrot %dx%d=%d, iters:%d, on %d cpus...\n", n, n, n * n, iters, Thread::nCpus());

  auto t0 = Timer();

  mandel.maneldebrot_mt();
  printf("mt: %ldms, writing mandel.png\n", t0.lap());

  mandel.writepng("mandel.png");

  t0.start();
  mandel.maneldebrot_st();
  printf("st: %ldms\n", t0.lap());
}

int main()
{
  test_mandel();

  return 0;
}