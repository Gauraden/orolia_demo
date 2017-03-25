#include <gtkmm.h>
#include <gtkmm/drawingarea.h>
#include <boost/format.hpp>
#include "demo_gui.hpp"

GuiSettings::GuiSettings()
    : draw_scales(true) {
}

class ChartArea : public Gtk::DrawingArea {
  public:
    ChartArea(const Compressor::ShrPtr &ptr, 
              const GuiSettings        &settings)
        : Gtk::DrawingArea(),
          _comp(ptr),
          _settings(settings) {
      auto layout = create_pango_layout("0.0");
      int text_width;
      int text_height;
      layout->get_pixel_size(text_width, text_height);
      _label_h = std::abs(text_height / 2);
    }
    virtual ~ChartArea() {}
  protected:
    typedef Cairo::RefPtr<Cairo::Context> ContextRef;

    bool on_draw(const ContextRef &ctx_ref) override {
      Gtk::Allocation allocation = get_allocation();
      _wnd_w = allocation.get_width();
      _wnd_h = allocation.get_height();
      DrawBackground(ctx_ref);
      if (_settings.draw_scales) {
        DrawScales(ctx_ref);
      }
      DrawGraph(ctx_ref);
      return true;
    }
  private:
    static const uint16_t kVPadding = 10;

    void DrawBackground(const ContextRef &ctx) {
      ctx->set_source_rgb(0.1, 0.1, 0.1);
      ctx->rectangle(0, 0, _wnd_w, _wnd_h);
      ctx->fill();
    }

    void DrawScales(const ContextRef &ctx) {
      auto records = _comp->GetRecords();
      const uint16_t kScaleLines  = 50;
      const uint16_t kScaleLabels = 10;
      const uint16_t kWStepSz   = _wnd_w / kScaleLines;
      const uint16_t kHStepSz   = _wnd_h / kScaleLines;
      for (auto w_off = kWStepSz; w_off < _wnd_w; w_off += kWStepSz) {
        ctx->move_to(w_off, 0);  
        ctx->line_to(w_off, _wnd_h);
      }
      for (auto h_off = kHStepSz; h_off < _wnd_h; h_off += kHStepSz) {
        ctx->move_to(0,      h_off);  
        ctx->line_to(_wnd_w, h_off);
      }
      ctx->set_source_rgb(0.12, 0.12, 0.12);
      ctx->stroke();
      // drawing labels on scales
      const auto kTimeStep = _comp->GetTimeScaleLen() / kScaleLabels;
      auto w_off = _label_h;
      ctx->set_source_rgb(0.2, 0.2, 0.2);
      for (auto tm_off = records.begin()->time.first; w_off < _wnd_w; tm_off += kTimeStep) {
        ctx->save();
        ctx->move_to(w_off, _wnd_h - 2);
        ctx->rotate(M_PI / -2);
        ctx->show_text(boost::str(boost::format("%.4f") % tm_off));
        ctx->restore();
        w_off += kWStepSz * (kScaleLines / kScaleLabels); 
      }
    }

    uint8_t RecToGraphPoints(const Compressor::Record &rec, uint16_t out[2][2]) {
      Compressor::Range ratio[2];
      const auto kNum = _comp->CastRecordToScales(rec, ratio);
      auto i = 0;
      for (; i < kNum; ++i) {
        out[i][0] = (uint16_t)(ratio[i].first * _wnd_w);
        out[i][1] = _wnd_h - (uint16_t)(ratio[i].second * (_wnd_h - kVPadding));
      }
      return i;
    }

    void DrawGraph(const ContextRef &ctx) {
      typedef Compressor::Record::List RecList;
      auto records = _comp->GetRecords();
      auto rec_it  = records.begin();
      uint16_t pt[2][2];
      uint8_t  prev_num = 0;
      for (; rec_it != records.end(); ++rec_it) {
        const auto kNum = RecToGraphPoints(*rec_it, pt);
        for (auto i = 0; i < kNum; ++i) {
          if (prev_num == 0) {
            ctx->move_to(pt[i][0], pt[i][1]);  
            prev_num++;
            continue;
          }
          ctx->line_to(pt[i][0], pt[i][1]);
        }
        prev_num = kNum;
      }
      ctx->set_source_rgb(1, (float)167 / 256, (float)9 / 256);
      ctx->stroke();
    }

    unsigned           _wnd_w;
    unsigned           _wnd_h;
    Compressor::ShrPtr _comp;
    GuiSettings        _settings;
    unsigned           _label_h;
};

void CreateWindowWithChart(const Compressor::ShrPtr &comp_ptr,
                           const GuiSettings        &settings) {
  int    args = 0;
  char **argv = 0;
  auto app = Gtk::Application::create(args, argv, "org.gtkmm.examples.base");
  Gtk::Window window;
  ChartArea   area(comp_ptr, settings);
  window.set_default_size(800, 600);
  window.add(area);
  area.show();
  app->run(window);
}