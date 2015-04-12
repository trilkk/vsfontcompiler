#include "ft_glyph.hpp"
#include "glyph_range.hpp"
#include "glyph_storage.hpp"
#include "sky_line.hpp"
#include "sky_line_fitter.hpp"
#include "gfx/image_png.hpp"
#include "thr/dispatch.hpp"

#include <boost/filesystem.hpp>
#include <boost/exception/diagnostic_information.hpp>
#include <boost/program_options.hpp>

namespace fs = boost::filesystem;
namespace po = boost::program_options;

/** Usage help string. */
static const char g_usage_front[] =
"vsfontcompiler [options] -o <output_file_base> <fontfiles>\n"
"\n"
"This program will compile a truetype font into a precalculated font texture\n"
"file usable with the Valve algorithm.\n"
"\n"
"The font files used as input will be iterated in the order specified in the\n"
"command line. The glyphs will be extracted from the first font file that\n"
"contains them, successive font files are used as a fallback.\n"
"\n"
"Segment names are:\n";

static const char g_usage_back[] =
"\n"
"'default' range represents common shapes that are hard to classify into any\n" 
"specific segment, but are commonly used anyway.\n" 
"\n";

/** Segments enabled normally. */
static const char *g_segments_enabled[] =
{
  "default",
  "ascii",
  "latin",
  "greek",
  "cyrillic",
  "hiragana",
  "katakana",
  NULL
};

/** Verbose or not. */
bool g_verbose = false;

/** Convenience typedef. */
typedef std::list<FtFaceSptr> FaceList;

/** Convenience typedef. */
typedef std::map<std::string, GlyphRange> RangeMap;

/** \brief Fit glyphs.
 *
 * \param slf Sky line fitter.
 */
static void fit_glyphs(SkyLineFitter &slf, GlyphStorage &storage)
{
  slf.queue(storage);

  thr::wait();
  thr::thr_quit();
}

/** \brief Perform rendering of all glyphs.
 *
 * \param ranges ranges to render.
 * \param fonts List of fonts.
 * \param target_size Size to aim to.
 */
static void queue_glyphs(RangeMap &ranges, GlyphStorage &storage, FaceList &fonts, unsigned target_size)
{
  BOOST_FOREACH(const RangeMap::value_type &vv, ranges)
  {
    vv.second.queue(storage, fonts, target_size);
  }
  thr::wait();
  thr::thr_quit();
}

/** \brief Main function.
 *
 * \param argc Argument count.
 * \param argv Argument data.
 * \return Program exit code.
 */
int main(int argc, char **argv)
{
  try
  {
    std::vector<std::string> font_names;
    FaceList fonts;
    GlyphRange extra_range;
    GlyphStorage glyphs;
    RangeMap ranges;
    fs::path output_path;
    float dropdown = 0.1f;
    unsigned precalc_size = 2048,
             target_size = 48;
    bool can_execute = true,
         opengl_coordinates = true,
         version_printed = false;

    ranges[std::string("default")] = GlyphRange();
    ranges[std::string("ascii")] = GlyphRange(static_cast<unsigned>(' '), static_cast<unsigned>('~'));
    ranges[std::string("latin")] = GlyphRange(0xc0, 0xff);
    ranges[std::string("greek")] = GlyphRange(0x370, 0x3ff);
    ranges[std::string("cyrillic")] = GlyphRange(0x410, 0x44f);
    ranges[std::string("hiragana")] = GlyphRange(0x3040, 0x309e);
    ranges[std::string("katakana")] = GlyphRange(0x30a0, 0x30fe);
    ranges[std::string("unified-ideograms")] = GlyphRange(0x4e00, 0x9fa5);
    ranges[std::string("hangul")] = GlyphRange(0xac00, 0xd7af);

    // Default glyph range has some very specific contents.
    {
      GlyphRange &default_range = ranges[std::string("default")];

      default_range.add(0x2026);
      default_range.add(0x25a0);
      default_range.add(0x25af);
    }

    // allow default ranges
    for(const char** ii = g_segments_enabled; (NULL != (*ii)); ++ii)
    {
      RangeMap::iterator iter = ranges.find(std::string(*ii));

      BOOST_ASSERT(ranges.end() != iter);
      
      iter->second.enable();
    }

    {
      std::string coordinate_string;
      {
        std::ostringstream sstr;
        sstr << "System to store texture coordinates in, possible values: directx, opengl (default: " <<
          (opengl_coordinates ? "opengl" : "directx") << ").";
        coordinate_string = sstr.str();
      }
      std::string dropdown_string;
      {
        std::ostringstream sstr;
        sstr << "Relative distance (of whole glyph) of font edge it takes to reduce alpha-test to 0 " <<
          "(default: " << dropdown << ").";
        dropdown_string = sstr.str();
      }
      std::string include_string;
      {
        std::ostringstream sstr;
        sstr << "Include a segment, may be specified multiple times. Segments included may be symbolic names, individual characters specified by their unicode number or unicode number ranges separated by a colon. Segments included by default are:";
        BOOST_FOREACH(const RangeMap::value_type &vv, ranges)
        {
          if(vv.second.isEnabled())
          {
            sstr << "\n  " << vv.first;
          }
        }
        include_string = sstr.str();
      }
      std::string precalc_size_string;
      {
        std::ostringstream sstr;
        sstr << "Size of glyph to use in calculation (default: " << precalc_size << ").";
        precalc_size_string = sstr.str();
      }
      std::string target_size_string;
      {
        std::ostringstream sstr;
        sstr << "Target resolution to crunch glyphs to (default: " << target_size << ").";
        target_size_string = sstr.str();       
      }

      po::options_description desc("Options");
      desc.add_options()
        ("all,a", "Enable all known named segments by default.")
        ("coordinates,c", po::value<std::string>(), coordinate_string.c_str())
        ("custom-range,a", po::value<std::string>(), "Add an additional custom glyph range (separate with a colon character) or an individual glyph.")
        ("dropdown,d", po::value<float>(), dropdown_string.c_str())
        ("empty,e", "Do not enable any segments by default")
        ("font,f", po::value< std::vector<std::string> >(), "Font input file.")
        ("help,h", "Print help text.")
        ("include,i", po::value<std::vector<std::string> >(), include_string.c_str())
        ("outfile,o", po::value<std::string>(), "Output file basename.")
        ("precalc-size,p", po::value<unsigned>(), precalc_size_string.c_str())
        ("revoke,r", po::value<std::vector<std::string> >(), "Specifically deny a segment from being included, may be specified multiple times (default: none).")
        ("target-size,t", po::value<unsigned>(), target_size_string.c_str())
        ("verbose,v", "Turn on verbose reporting.")
        ("version,V", "Print version string");

      po::positional_options_description pdesc;
      pdesc.add("font", -1);

      po::variables_map vmap;
      po::store(po::command_line_parser(argc, argv).options(desc).positional(pdesc).run(), vmap);
      po::notify(vmap);

      if(vmap.count("coordinates"))
      {
        std::string coordinate_system = vmap["coordinates"].as<std::string>();
        if(coordinate_system.compare("opengl"))
        {
          opengl_coordinates = true;
        }
        else if(coordinate_system.compare("directx"))
        {
          opengl_coordinates = false;
        }
        else
        {
          std::stringstream err;
          err << "invalid coordinate system: " << coordinate_system;
          BOOST_THROW_EXCEPTION(std::runtime_error(err.str()));
        }
      }
      if(vmap.count("dropdown"))
      {
        dropdown = vmap["dropdown"].as<float>();
        if((0.0f >= dropdown) || (1.0f <= dropdown))
        {
          std::stringstream err;
          err << "invalid distance scale" << dropdown;
          BOOST_THROW_EXCEPTION(std::runtime_error(err.str()));
        }
      }
      if(vmap.count("font"))
      {
        font_names = vmap["font"].as< std::vector<std::string> >();
      }
      if((1 >= argc) || vmap.count("help"))
      {
        std::cout << g_usage_front;
        BOOST_FOREACH(const RangeMap::value_type &vv, ranges)
        {
          std::cout << "  " << vv.first << std::endl;
        }
        std::cout << g_usage_back << desc << std::endl;
        return 0;
      }
      if(vmap.count("outfile"))
      {
        if(output_path.generic_string().length() > 0)
        {
          std::stringstream err;
          err << "output files already specified";
          BOOST_THROW_EXCEPTION(std::runtime_error(err.str()));
        }
        output_path = fs::path(vmap["outfile"].as<std::string>());
        if(output_path.generic_string().length() <= 0)
        {
          std::stringstream err;
          err << "invalid output file specification " << output_path;
          BOOST_THROW_EXCEPTION(std::runtime_error(err.str()));
        }
      }
      if(vmap.count("precalc-size"))
      {
        precalc_size = vmap["precalc-size"].as<unsigned>();
        if(precalc_size <= 0)
        {
          std::stringstream err;
          err << "invalid precalculation size" << precalc_size;
          BOOST_THROW_EXCEPTION(std::runtime_error(err.str()));
        }
      }
      if(vmap.count("target-size"))
      {
        target_size = vmap["target-size"].as<unsigned>();
        if(target_size <= 0)
        {
          std::stringstream err;
          err << "invalid crunch size" << target_size;
          BOOST_THROW_EXCEPTION(std::runtime_error(err.str()));
        }
      }
      if(vmap.count("verbose"))
      {
        g_verbose = true;
      }
      if(vmap.count("version"))
      {
        std::cout << VERSION << std::endl;
        version_printed = true;
      }

      bool enable_all = (0 < vmap.count("all")),
           enable_none = (0 < vmap.count("empty"));
      if(enable_all && enable_none)
      {
        std::ostringstream sstr;
        sstr << "both 'all' and 'empty' options specified at the same time";
        BOOST_THROW_EXCEPTION(std::runtime_error(sstr.str()));
      }
      else if(enable_all)
      {
        BOOST_FOREACH(RangeMap::value_type &vv, ranges)
        {
          vv.second.enable();
        }
      }
      else if(enable_none)
      {
        BOOST_FOREACH(RangeMap::value_type &vv, ranges)
        {
          vv.second.disable();
        }
      }

      if(vmap.count("include"))
      {
        std::vector<std::string> segments = vmap["include"].as<std::vector<std::string> >();
        BOOST_FOREACH(const std::string &ii, segments)
        {
          RangeMap::iterator iter = ranges.find(ii);
          if(ranges.end() != iter)
          {
            iter->second.enable();
          }
          else
          {
            unsigned uu1, uu2;

            if(2 == sscanf(ii.c_str(), "%u:%u", &uu1, &uu2))
            {
              extra_range.add(uu1, uu2);
              extra_range.enable();
            }
            else if(1 == sscanf(ii.c_str(), "%u", &uu1))
            {
              extra_range.add(uu1);
              extra_range.enable();
            }
            else
            {
              std::ostringstream sstr;
              sstr << "invalid range description: '" << ii << '\'';
              BOOST_THROW_EXCEPTION(std::runtime_error(sstr.str()));
            }
          }
        }
      }
      if(vmap.count("revoke"))
      {
        std::vector<std::string> segments = vmap["revoke"].as<std::vector<std::string> >();
        BOOST_FOREACH(const std::string &ii, segments)
        {
          RangeMap::iterator iter = ranges.find(ii);
          if(ranges.end() == iter)
          {
            iter->second.disable();
          }
          else
          {
            unsigned uu1, uu2;

            if(2 == sscanf(ii.c_str(), "%u:%u", &uu1, &uu2))
            {
              BOOST_FOREACH(RangeMap::value_type &vv, ranges)
              {
                vv.second.remove(uu1, uu2);
              }
              extra_range.remove(uu1, uu2);
            }
            else if(1 == sscanf(ii.c_str(), "%u", &uu1))
            {
              BOOST_FOREACH(RangeMap::value_type &vv, ranges)
              {
                vv.second.remove(uu1);
              }
              extra_range.remove(uu1);
            }
            else
            {
              std::ostringstream sstr;
              sstr << "invalid range description: '" << ii << '\'';
              BOOST_THROW_EXCEPTION(std::runtime_error(sstr.str()));
            }
          }
        }
      }
    }

    // perform sanity checks
    if(output_path.generic_string().length() <= 0)
    {
      can_execute = false;

      if(!version_printed)
      {
        std::stringstream err;
        err << "output files not specified";
        BOOST_THROW_EXCEPTION(std::runtime_error(err.str()));
      }
    }
    else if(g_verbose)
    {
      std::cout << "Using output file base: " << output_path << std::endl;
    }

    if(font_names.empty())
    {
      can_execute = false;

      if(!version_printed)
      {
        BOOST_THROW_EXCEPTION(std::runtime_error("no valid font files"));
      }
    }

    if(extra_range.isEnabled())
    {
      ranges[std::string("extra")] = extra_range;
    }

    if(!can_execute)
    {
      return 0;
    }

    // load fonts
    BOOST_FOREACH(std::string &vv, font_names)
    {
      fonts.push_back(boost::shared_ptr<FtFace>(new FtFace(vv, precalc_size, dropdown)));
    }

    // Perform the actual generation of the glyphs.
    if(g_verbose)
    {
      std::cout << "Rendering:";
      std::cout.flush();
    }
    thr::thr_init();
    {
      boost::thread render_thread(boost::bind(queue_glyphs, boost::ref(ranges), boost::ref(glyphs), boost::ref(fonts), target_size));
      thr::thr_main();
    }
    glyphs.sort();

    // Open the XML file and write the header.
    std::string xmlfilename(output_path.generic_string() + std::string(".xml"));
    FILE *xmlfile = fopen(xmlfilename.c_str(), "wt");
    if(!xmlfile)
    {
      std::stringstream err;
      err << "could not open " << xmlfilename << "for writing";
      BOOST_THROW_EXCEPTION(std::runtime_error(err.str()));
    }
    fputs("<?xml version=\"1.0\" encoding=\"utf-8\"?>\n"
        "<font xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" "
        "xmlns:xsd=\"http://www.w3.org/2001/XMLSchema\">\n",
        xmlfile);

    // Perform fitting along the skyline algorithm.
    for(unsigned image_index = 0; (!glyphs.empty()); ++image_index)
    {
      if(g_verbose)
      {
        std::cout << std::endl << ((0 >= image_index) ? "Start" : "Continue") << " fitting process: " <<
          glyphs.size() << " glyphs left\n";
      }

      SkyLineFitter slf(2048);
      {
        boost::thread fit_thread(boost::bind(fit_glyphs, boost::ref(slf), boost::ref(glyphs)));
        thr::thr_main();
      }

      SkyLine sl(slf.getBestWidth(), slf.getBestHeight());

      sl.fitAll(glyphs, xmlfile, image_index, opengl_coordinates);

      glyphs.trim(); // will also sort

      std::ostringstream sstr;
      sstr << output_path.generic_string() << '_' << image_index << ".png";
      fs::path pngfilepath(sstr.str());
      std::string pngfilename(pngfilepath.generic_string());

      fprintf(xmlfile, "\t<texture>%s</texture>\n", pngfilename.c_str());
      sl.save(pngfilename);
    }

    // Close the XML file.
    fputs("</font>", xmlfile);
    fclose(xmlfile);

    if(g_verbose)
    {
      std::cout << "\nDone.\n";
    }
  }
  catch(const boost::exception &err)
  {
    std::cerr << boost::diagnostic_information(err);
    return 1;
  }
  catch(...)
  {
    std::cerr << "Unkown exception caught!" << std::endl;
    return -1;
  }

  return 0;
}

