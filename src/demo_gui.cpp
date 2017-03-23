#include <gtkmm.h>
#include <gtkmm/drawingarea.h>
#include <gdkmm/pixbuf.h>
#include "demo_gui.hpp"

class ChartArea : public Gtk::DrawingArea {
  public:
    ChartArea(): Gtk::DrawingArea() {}
    virtual ~ChartArea() {}
  protected:
    typedef Cairo::RefPtr<Cairo::Context> ContextRef;

    bool on_draw(const ContextRef &ctx_ref) override {
      Gtk::Allocation allocation = get_allocation();
      _wnd_w = allocation.get_width();
      _wnd_h = allocation.get_height();
      DrawBackground(ctx_ref);
      DrawScale(ctx_ref);
      DrawGraph(ctx_ref);
    }
  private:
    void DrawBackground(const ContextRef &ctx) {
      ctx->set_source_rgb(0.1, 0.1, 0.1);
      ctx->rectangle(0, 0, _wnd_w, _wnd_h);
      ctx->fill();
    }

    void DrawScale(const ContextRef &ctx) {

    }

    void DrawGraph(const ContextRef &ctx) {

    }

    unsigned _wnd_w;
    unsigned _wnd_h;
};

void CreateWindow(int arg_amount, char **arg_values) {
  auto app = Gtk::Application::create(arg_amount, arg_values,
      "org.gtkmm.examples.base");

  Gtk::Window window;
  ChartArea   area;
  window.set_default_size(800, 600);
  window.add(area);
  area.show();
  app->run(window);
}