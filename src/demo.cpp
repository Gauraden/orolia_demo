#include <string>
#include <iostream>
#include <memory>
#include <boost/program_options.hpp>
#include <gtkmm.h>
#include <gtkmm/drawingarea.h>
#include <gdkmm/pixbuf.h>

class DataSource {
  public:
    typedef std::shared_ptr<DataSource> Ptr;
    DataSource() {}
    virtual ~DataSource() {}
    virtual bool SelectAll() = 0;
};

class TextFile : public DataSource {
  public:
    TextFile(const std::string &path): DataSource(), _path(path) {}
    virtual ~TextFile() {}
    static DataSource::Ptr Create(const std::string &path) {
      return DataSource::Ptr(new TextFile(path));
    }
    virtual bool SelectAll() {

    }
  private:
  const std::string _path;
};

class ChartArea : public Gtk::DrawingArea {
  public:
    ChartArea(DataSource::Ptr &&ptr): Gtk::DrawingArea(), _data_src(ptr) {}
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

    unsigned        _wnd_w;
    unsigned        _wnd_h;
    DataSource::Ptr _data_src;
};

static
bool ParseProgramArguments(int arg_amount, char **arg_values) {
  namespace po = boost::program_options;
  po::options_description desc("Demo program for OROLIA");
	desc.add_options()
		("help", "this description")
    ("in",   po::value<std::string>()->default_value(""),
             "path to file with source data");
	po::variables_map vm;
	po::store(po::parse_command_line(arg_amount, arg_values, desc), vm);
	po::notify(vm);
	if (vm.count("help")) {
	  std::cout << desc << std::endl;
	  return false;
	}

  return true;
}

static
void CreateWindow(int arg_amount, char **arg_values) {
  auto app = Gtk::Application::create(arg_amount, arg_values,
      "org.gtkmm.examples.base");

  Gtk::Window window;
  ChartArea   area(TextFile::Create(""));
  window.set_default_size(800, 600);
  window.add(area);
  area.show();
  app->run(window);
}

int main(int arg_amount, char **arg_values) {
  if (not ParseProgramArguments(arg_amount, arg_values)) {
    return 0;
  }
  CreateWindow(arg_amount, arg_values);
  return 0;
}