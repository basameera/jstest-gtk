/*
**  jstest-gtk - A graphical joystick tester
**  Copyright (C) 2009 Ingo Ruhnke <grumbel@gmail.com>
**
**  This program is free software: you can redistribute it and/or modify
**  it under the terms of the GNU General Public License as published by
**  the Free Software Foundation, either version 3 of the License, or
**  (at your option) any later version.
**
**  This program is distributed in the hope that it will be useful,
**  but WITHOUT ANY WARRANTY; without even the implied warranty of
**  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**  GNU General Public License for more details.
**
**  You should have received a copy of the GNU General Public License
**  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <iostream>
#include <gtkmm.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "joystick_test_widget.hpp"
#include "joystick_list_widget.hpp"
#include "joystick_map_widget.hpp"
#include "joystick_calibration_widget.hpp"
#include "joystick.hpp"
#include "main.hpp"

Main* Main::current_ = 0;

Main::Main() :
  Gtk::Application("com.gmail.grumbel.jstest-gtk", Gio::APPLICATION_HANDLES_OPEN),
  datadir("data/"),
  m_simple_ui(false)
{
  current_ = this;
}

Main::~Main()
{
}

JoystickTestWidget*
Main::show_device_property_dialog(const std::string& filename, Gtk::Window* parent)
{
  Joystick* joystick = new Joystick(filename);
  JoystickTestWidget* dialog = new JoystickTestWidget(*joystick, m_simple_ui);
  if (parent) {
    dialog->set_transient_for(*parent);
  }

  dialog->signal_hide().connect(sigc::bind(sigc::mem_fun(this, &Main::on_dialog_hide), dialog));
  dialog->show_all();

  joysticks.push_back(joystick);
  dialogs.push_back(dialog);

  return dialog;
}

void
Main::show_calibration_dialog(Joystick& joystick)
{
  JoystickCalibrationWidget* dialog = new JoystickCalibrationWidget(joystick);
  dialog->signal_hide().connect(sigc::bind(sigc::mem_fun(this, &Main::on_dialog_hide), dialog));
  dialog->show_all();
  dialogs.push_back(dialog);
}

void
Main::show_mapping_dialog(Joystick& joystick)
{
  JoystickMapWidget* dialog = new JoystickMapWidget(joystick);
  dialog->signal_hide().connect(sigc::bind(sigc::mem_fun(this, &Main::on_dialog_hide), dialog));
  dialog->show_all();
  dialogs.push_back(dialog);
}

void
Main::on_dialog_hide(Gtk::Dialog* dialog)
{
  dialogs.erase(std::remove(dialogs.begin(), dialogs.end(), dialog), dialogs.end());
  delete dialog;
}

int
Main::run(int argc, char** argv)
{
  typedef std::vector<std::string> DeviceFiles;
  DeviceFiles device_files;

  for(int i = 1; i < argc; ++i)
  {
    if (strcmp("--help", argv[i]) == 0 ||
        strcmp("-h", argv[i]) == 0)
    {
      std::cout << "Usage: " << argv[0] << " [OPTIONS]... [DEVICE]\n"
                << "A graphical joystick tester.\n"
                << "\n"
                << "Options:\n"
                << "  -h, --help      Display this help and exit\n"
                << "  -v, --version   Display version information and exit\n"
                << "  --simple        Hide graphical representation of axis\n"
                << "  --datadir DIR   Load application data from DIR\n"
                << "\n"
                << "Report bugs to Ingo Ruhnke <grumbel@gmail.com>.\n";
      return 0;
    }
    else if (strcmp("--version", argv[i]) == 0 ||
             strcmp("-v", argv[i]) == 0)
    {
      std::cout << "jstest-gtk 0.1.0" << std::endl;
      return 0;
    }
    else if (strcmp("--simple", argv[i]) == 0)
    {
      m_simple_ui = true;
    }
    else if (strcmp("--datadir", argv[i]) == 0)
    {
      i += 1;
      if (i >= argc)
      {
        std::cout << "Error: " << argv[0] << ": argument to --datadir is missing" << std::endl;
        return EXIT_FAILURE;
      }
      else
      {
        datadir = argv[i];
      }
    }
    else if (argv[i][0] == '-')
    {
      std::cout << "Error: " << argv[0] << ": unrecognized option '" << argv[i] << "'" << std::endl;
      return EXIT_FAILURE;
    }
    else
    {
      if (!device_files.empty())
      {
        std::cout << "Error: " << argv[0] << ": multiple device files given, only one allowed: " << argv[i] << std::endl;
        return EXIT_FAILURE;
      }

      device_files.push_back(argv[i]);
    }
  }

  try
  {
    if (device_files.empty())
    {
      JoystickListWidget list_dialog;
      list_dialog.show_all();
      return Gtk::Application::run(list_dialog);
    }
    else
    {
      auto dialog = show_device_property_dialog(*device_files.begin());
      return Gtk::Application::run(*dialog);
    }
  }
  catch(std::exception& err)
  {
    std::cout << "Error: " << err.what() << std::endl;
    return EXIT_FAILURE;
  }
  catch(Glib::Exception& err)
  {
    std::cout << "Error: " << err.what() << std::endl;
    return EXIT_FAILURE;
  }

  return 0;
}

int main(int argc, char** argv)
{
  try
  {
    Glib::RefPtr<Main> app = Main::create();

    return app->run(argc, argv);
  }
  catch(std::exception& err)
  {
    std::cout << "Error: " << err.what() << std::endl;
    return EXIT_FAILURE;
  }
}

Glib::RefPtr<Main> Main::create()
{
  return Glib::RefPtr<Main>(new Main);
}

/* EOF */
