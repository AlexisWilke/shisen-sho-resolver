// this tool:
//
// 1. loads a shisen-sho screenshot
// 2. converts it to a 2-dimensional array of tiles
// 3. checks that the game can be resolved


// to debug with a full stack of all boards (should use up to 68 copies
// so probably doable even on computers with not much memory)
//
#define DEBUG_WITH_STACK


// ImageMagick
// At the moment, we only have Q16 on Ubuntu, so no choice here...
//
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wignored-qualifiers"
#pragma GCC diagnostic ignored "-Wpedantic"
#pragma GCC diagnostic ignored "-Wredundant-decls"
#define MAGICKCORE_QUANTUM_DEPTH 16
#define MAGICKCORE_HDRI_ENABLE 0
#include    <Magick++.h>
#pragma GCC diagnostic pop

// C++
//
#include    <cstring>
#include    <fstream>
#include    <iomanip>
#include    <iostream>
#include    <memory>
#include    <sstream>



// if you need to regenerate, change the #if below
//
#if 1
#include    "tiles.cpp"
#else
// original sizes come from the first sample
constexpr int const ORIGINAL_TILE_WIDTH = 95;
constexpr int const ORIGINAL_TILE_HEIGHT = 131;
constexpr std::uint8_t const g_tiles[] = { 1 };
#endif



class resolver
{
public:
    int             parse_args(int argc, char * argv[]);

    int             run();

private:
    typedef std::int8_t                     tile_t;

    // (short) key is board_position(column, row);
    // (tile_t) value is the tile number
    //
    typedef std::map<short, tile_t>         board_t;
    typedef std::shared_ptr<board_t>        board_pointer_t;
    typedef std::vector<board_pointer_t>    board_vector_t;

    enum direction_t : std::uint8_t
    {
        DIRECTION_NONE,

        DIRECTION_LEFT,
        DIRECTION_UP,
        DIRECTION_RIGHT,
        DIRECTION_DOWN,
    };
    struct position_t
    {
        typedef std::list<position_t>               list_t;
        typedef std::map<tile_t, list_t>            map_t;  // key is item number (1 to 36 in our case)

        direction_t f_direction = DIRECTION_NONE;
        //int8_t      f_turns = -1;
        std::int8_t f_column = -1;
        std::int8_t f_row = -1;

        bool same_position(position_t const & p) const
        {
            return f_column == p.f_column && f_row == p.f_row;
        }
    };
    struct step_t
    {
        typedef std::shared_ptr<step_t> pointer_t;
        typedef std::vector<pointer_t>  vector_t;

        pointer_t   f_parent = pointer_t();
        position_t  f_p1 = {};
        position_t  f_p2 = {};
        vector_t    f_children = vector_t();
        tile_t      f_tile = 0;
    };

    void                usage();
    void                setup_original_tiles();
    int                 load_image();
    int                 load_board();
    int                 image_to_board();
    int                 save_board(bool save_image = false);
    position_t::list_t  find_matches(position_t const & p1);
    int                 resolve_board();
    bool                is_tile(
                              Magick::PixelPacket const * ptr
                            , int x
                            , int y
                            , int width
                            , int height) const;
    int                 find_right_edge(
                              Magick::PixelPacket const * ptr
                            , int x
                            , int y
                            , int width
                            , int height) const;
    int                 find_bottom_edge(
                              Magick::PixelPacket const * ptr
                            , int x
                            , int y
                            , int width
                            , int height) const;
    int                 find_tile(
                              Magick::PixelPacket const * ptr
                            , int x
                            , int y
                            , int width
                            , int height
                            , int tile_width
                            , int tile_height);
    void                add_tile(
                              Magick::PixelPacket const * ptr
                            , int x
                            , int y
                            , int width
                            , int height
                            , int tile_width
                            , int tile_height);
    void                output_tile(
                              std::ofstream & generate_out
                            , Magick::PixelPacket const * ptr
                            , int x
                            , int y
                            , int width
                            , int height
                            , int tile_width
                            , int tile_height);
    int                 board_position(
                              int column
                            , int row);
    int                 board_position(position_t const & p);
    bool                next_position(position_t & p);
    void                remove_tile(position_t const & p);
    void                restore_tile(position_t const & p, tile_t tile);
    step_t::vector_t    match_tiles(position_t const & p1);
    bool                find_connection(position_t p1, position_t p2);
    bool                connect_left(position_t p1, position_t p2);
    bool                connect_right(position_t p1, position_t p2);
    bool                connect_down(position_t p1, position_t p2);
    bool                connect_up(position_t p1, position_t p2);
    bool                connect_up_left(position_t p1, position_t p2);
    bool                connect_up_right(position_t p1, position_t p2);
    bool                connect_right_down(position_t p1, position_t p2);
    bool                connect_left_down(position_t p1, position_t p2);
    bool                connect_left_up_left(position_t p1, position_t p2);
    bool                connect_right_up_right(position_t p1, position_t p2);
    bool                connect_right_down_right(position_t p1, position_t p2);
    bool                connect_left_down_left(position_t p1, position_t p2);
    bool                connect_left_up_right(position_t p1, position_t p2);
    bool                connect_right_up_left(position_t p1, position_t p2);
    bool                connect_up_right_down(position_t p1, position_t p2);
    bool                connect_up_left_down(position_t p1, position_t p2);
    bool                connect_down_left_or_right_up(position_t p1, position_t p2);

    std::string         f_filename = std::string();
    std::string         f_save_board = std::string();
    std::string         f_save_steps = std::string();
    std::string         f_generate_tiles = std::string();
    bool                f_resolve_board = true;

    Magick::Image       f_image = Magick::Image();
    std::vector<std::uint8_t const *> // this is the RGB data
                        f_tiles = std::vector<std::uint8_t const *>();
    int                 f_tile_width = 0;
    int                 f_tile_height = 0;
    int                 f_columns = 127;
    int                 f_rows = 127;
    board_t             f_board = board_t();
    board_vector_t      f_board_resolved = board_vector_t();
    board_vector_t      f_board_stack = board_vector_t();       // for debug only (to make sure remove + restore works)
    step_t::pointer_t   f_first_step = step_t::pointer_t();     // this is the root, it does not represent a step per se
};


void resolver::usage()
{
    std::cout << "Usage: shisen-sho-resolver [-opts] <screenshot>\n";
    std::cout << "where [-opts] is zero or more of:\n";
    std::cout << "  -h | --help               print our this help screen\n";
    std::cout << "  --save-board <filename>   if specified, save original board\n";
    std::cout << "  --save-steps <filename>   if specified, save steps to resolve board\n";
    std::cout << "  --save-board-only         use to load board, save to image, then quit\n";
}


int resolver::parse_args(int argc, char * argv[])
{
    for(int i(1); i < argc; ++i)
    {
        if(argv[i][0] == '-')
        {
            if(strcmp(argv[i], "-h") == 0
            || strcmp(argv[i], "--help") == 0)
            {
                usage();
                return 1;
            }
            else if(strcmp(argv[i], "--save-board") == 0)
            {
                ++i;
                if(i >= argc)
                {
                    std::cerr << "error: --save-board must be followed by a filename.\n";
                    return 1;
                }
                f_save_board = argv[i];
            }
            else if(strcmp(argv[i], "--save-steps") == 0)
            {
                ++i;
                if(i >= argc)
                {
                    std::cerr << "error: --save-steps must be followed by a filename.\n";
                    return 1;
                }
                f_save_steps = argv[i];
            }
            else if(strcmp(argv[i], "--generate-tiles") == 0)
            {
                ++i;
                if(i >= argc)
                {
                    std::cerr << "error: --generate-tiles must be followed by a filename.\n";
                    return 1;
                }
                f_generate_tiles = argv[i];
            }
            else if(strcmp(argv[i], "--save-board-only") == 0)
            {
                f_resolve_board = false;
            }
            else
            {
                std::cerr << "error: unrecognized command line option \"" << argv[i] << "\".\n";
                return 1;
            }
        }
        else if(!f_filename.empty())
        {
            std::cerr << "error: only the name of the input image file can be specified standalone.\n";
            return 1;
        }
        else
        {
            f_filename = argv[i];
        }
    }

    if(f_filename.empty())
    {
        std::cerr << "error: an input filename is required.\n";
        return 1;
    }

    return 0;
}


int resolver::run()
{
    int r(0);

    if(f_generate_tiles.empty())
    {
        setup_original_tiles();
    }

    if(f_filename.ends_with(".txt"))
    {
        r = load_board();
        if(r != 0)
        {
            return r;
        }

        r = save_board(true);
        if(r != 0)
        {
            return r;
        }
    }
    else
    {
        r = load_image();
        if(r != 0)
        {
            return r;
        }

        r = image_to_board();
        if(r != 0)
        {
            return r;
        }

        if(f_columns == 127 || f_rows == 127)
        {
            throw std::logic_error("rows and/or columns were not properly determined while scanning the image.");
        }

        r = save_board(false);
        if(r != 0)
        {
            return r;
        }
    }

    if(f_resolve_board)
    {
        r = resolve_board();
        if(r != 0)
        {
            return r;
        }

        //r = save_steps();
        if(r != 0)
        {
            return r;
        }
    }

    return 0;
}


void resolver::setup_original_tiles()
{
    std::uint8_t const * end(g_tiles + sizeof(g_tiles));
    for(std::uint8_t const * t(g_tiles);
        t < end;
        t += ORIGINAL_TILE_WIDTH * ORIGINAL_TILE_HEIGHT * 3 + 1)
    {
        f_tiles.push_back(t);
    }
//std::cerr << "original tiles: " << f_tiles.size() << "\n";
}


int resolver::load_image()
{
    f_image.read(f_filename);

#if 0
// verify
std::cout << "save raw image data\n";
    Magick::PixelPacket * ptr(f_image.getPixels(
                      0
                    , 0
                    , f_image.columns()
                    , f_image.rows()));
    std::ofstream out("image.raw");
    out.write(reinterpret_cast<char const *>(ptr), f_image.columns() * f_image.rows() * 8);
#endif

    return 0;
}


int resolver::load_board()
{
    std::ifstream in;
    in.open(f_filename);
    if(!in.is_open())
    {
        std::cerr
            << "error: could not open file \""
            << f_filename
            << "\".\n";
        return 1;
    }

    in.seekg(0, std::ios::end);
    std::size_t const size(in.tellg());
    in.seekg(0, std::ios::beg);

    std::vector<char> contents(size);
    char * s(contents.data());
    char const * e(s + size);
    in.read(s, size);
    while(s < e && (isspace(*s) || *s == ','))
    {
        ++s;
    }
    if(s >= e)
    {
        throw std::runtime_error("s reached e before finding any tile.");
    }

    position_t p;
    p.f_column = 0;
    p.f_row = 0;
    while(s < e)
    {
        char const * const start(s);
        int tile_no(0);
        while(s < e && *s >= '0' && *s <= '9')
        {
            tile_no = tile_no * 10 + *s - '0';
            ++s;
        }
        if(s == start)
        {
            throw std::runtime_error("no number found here.");
        }
        f_board[board_position(p)] = tile_no;
        if(*s == '\n' && f_columns == 127)
        {
            f_columns = p.f_column + 1;
        }
        next_position(p);
        while(s < e && (isspace(*s) || *s == ','))
        {
            ++s;
        }
    }
    if(p.f_column != 0)
    {
        throw std::runtime_error("found the end of the file, but the column is not 0.");
    }
    f_rows = p.f_row;

    return 0;
}


int resolver::image_to_board()
{
    // get direct pointer to pixels
    // the format is 16 bits per component and RGBO
    // (where the O is Opacity, O = 1 - A and we can ignore it entirely)
    //
    int const width(f_image.columns());
    int const height(f_image.rows());
    Magick::PixelPacket * ptr(f_image.getPixels(
                      0
                    , 0
                    , width
                    , height));
//std::cerr << "size " << width << "x" << height << "\n";

    // the board is squares (or rectanlges?) of tiles arranged in an array of
    // X x Y tiles; one tile has a size: W x H; we search for a first square
    // of about 8x8 pixels that look white; that is expected to represent the
    // top-left corner of the title; then we deterimine the width and height
    // and transform that in an image to compare with our set of tiles to
    // determine its type; then we repeat for all the tiles (X x Y) to
    // generate the entire board
    //

    // if generating, we're not actually "loading the board", instead, we
    // get all the tiles and generate a .cpp instead
    //
    std::ofstream generate_out;
    if(!f_generate_tiles.empty())
    {
        generate_out.open(f_generate_tiles);
        if(!generate_out.is_open())
        {
            std::cerr << "error: could not open generated output file.\n";
            return 1;
        }
        generate_out << "// AUTO-GENERATED SET OF TILES\n";
    }

    // the edges of a tiles are mostly white near the top and go darker
    // toward the bottom; the top-left 8x8 square is 99% white so we search
    // for those and then determine the rectangle
    //
    int row(0);
    int column(0);
    int x(0);
    int y(0);
    for(;;)
    {
        if(!is_tile(ptr, x, y, width, height))
        {
            ++x;
            if(x >= width - 16)
            {
                ++y;
                x = 0;
            }
            continue;
        }

        // we found a top-left corner, first determine the width
        //
        int const right(find_right_edge(ptr, x, y, width, height));
        int const bottom(find_bottom_edge(ptr, x - 3, y, width, height));
        if(right < x
        || bottom < y)
        {
            std::cerr << "error: seaching edges (right: " << right << ", bottom: " << bottom << ")\n";
            return 1;
        }

        // retrieve the tile in a separate image
        //
        int const tile_width(right - x - 3);
        int const tile_height(bottom - y);
//std::cerr << "tile size " << tile_width << " / " << tile_height << " -- " << column << ", " << row << "\n";
//if(tile_height < 100)
//{
//std::cerr << "WARNING: bottom not found!? " << f_tiles.size() << "\n";
//}

        if(!f_generate_tiles.empty())
        {
            // in this case we save the tiles to disk so we can use
            // them to compare and determine what tile it is in our
            // C++ app.
            //
            // we want to save only one sample per tile, so we want
            // to save the image in a list and search the list before
            // saving another copy
            //
            // the output is a .cpp file that we can compile and use
            // here to search tiles when not generating this file
            //
#if 1
            {
std::cerr << "saving tile for verification\n";
                static bool create_dir = true;
                if(create_dir)
                {
                    create_dir = false;
                    int const r(system("mkdir -p tiles"));
                    if(r != 0)
                    {
                        std::cerr << "failed creating \"./tiles/\" sub-folder trying to save tiles for verification\n";
                        return 1;
                    }
                }
                std::string filename("tiles/");
                filename += std::to_string(column);
                filename += '-';
                filename += std::to_string(row);
                //filename += std::to_string(f_tiles.size());
                filename += ".png";

                Magick::Image tile(f_image);
                tile.crop(Magick::Geometry(tile_width, tile_height, x, y));
                try
                {
                    // PNG doesn't like grayscale (don't ask!?)
                    tile.write(filename);
                }
                catch(Magick::WarningCoder const &)
                {
                    // ImageMagick will pass grayscale images as is to libpng
                    // which generates an error. So we have to try again forcing
                    // RGB. Forcing RGB all the time breaks the colors in the
                    // colored images (who knows why this is so complicated?!)
                    //
                    tile.colorSpace(Magick::RGBColorspace);
                    tile.write(filename);
                }
            }
#endif
            if(f_tile_width == 0
            || f_tile_height == 0)
            {
                f_tile_width = tile_width;
                f_tile_height = tile_height;

                generate_out
                    << "constexpr int const ORIGINAL_TILE_WIDTH = " << f_tile_width << ";\n"
                    << "constexpr int const ORIGINAL_TILE_HEIGHT = " << f_tile_height << ";\n"
                    << "constexpr std::uint8_t const g_tiles[] = {\n";
            }
            if(find_tile(ptr, x, y, width, height, tile_width, tile_height) == 0)
            {
                add_tile(ptr, x, y, width, height, tile_width, tile_height);
                output_tile(generate_out, ptr, x, y, width, height, tile_width, tile_height);
            }
        }
        else
        {
            if(f_tile_width == 0
            || f_tile_height == 0)
            {
                f_tile_width = ORIGINAL_TILE_WIDTH;
                f_tile_height = ORIGINAL_TILE_HEIGHT;
            }

            // search our list of tiles to find which one we just found
            //
            Magick::Image tile(f_image);
            tile.crop(Magick::Geometry(tile_width, tile_height, x, y));
            tile.sample(Magick::Geometry(f_tile_width, f_tile_height, 0, 0));
            Magick::PixelPacket * tile_ptr(tile.getPixels(
                  0
                , 0
                , f_tile_width
                , f_tile_height));
            int const tile_no(find_tile(
                  tile_ptr
                , 0
                , 0
                , f_tile_width
                , f_tile_height
                , f_tile_width
                , f_tile_height));
            if(tile_no == 0)
            {
                std::cerr << "error: could not determine tile at ("
                    << x
                    << ", "
                    << y
                    << ") with size ("
                    << tile_width
                    << ", "
                    << tile_height
                    << ").\n";
                return 1;
            }

            f_board[board_position(column, row)] = tile_no;
//std::cerr << "(" << column << ", " << row << ") = " << tile_no << "\n";
        }

        // skip the tile
        //
        x = right;
        if(x + tile_width >= width)
        {
            x = 0;
            y = bottom;
            if(y + tile_height >= height)
            {
                if(!f_generate_tiles.empty())
                {
                    generate_out << "};\n";
                }
                f_rows= row + 1;
                return 0;
            }
            if(f_columns == 127)
            {
                f_columns = column + 1;
            }
            else if(f_columns != column + 1)
            {
                std::cerr
                    << "error: row "
                    << row
                    << " has "
                    << column
                    << " columns when previous rows had "
                    << f_columns
                    << ".\n";
                return 1;
            }
            ++row;
            column = 0;
        }
        else
        {
            ++column;
        }
    }

    throw std::logic_error("exited tile loop");
}


int resolver::board_position(int column, int row)
{
    if(column < 0 || column >= f_columns
    || row < 0 || row >= f_rows)
    {
        throw std::runtime_error("column and/or row out of bound.");
    }
    return column + (row << 8);
}


int resolver::board_position(position_t const & p)
{
    return board_position(p.f_column, p.f_row);
}


int resolver::save_board(bool save_image)
{
    if(f_save_board.empty())
    {
        // this is not an error, just no save if no name specified
        //
        return 0;
    }

    std::string output_filename;
    if(f_filename.ends_with(".txt"))
    {
        output_filename = "/dev/null";
    }
    else
    {
        output_filename = f_save_board;
    }

std::cerr << "--- saving to [" << output_filename << "]!?\n";
    std::ofstream out(output_filename);
    if(!out.is_open())
    {
        std::cerr
            << "error: could not open output file \""
            << f_save_board
            << "\" to save board.\n";
        return 1;
    }

    // generate an RGB raw image buffer as well
    // (the tiles are cut a bit, but it's easy to read compared to just
    // numbers that do not match the tiles in any way)
    //
    Magick::Color black(0, 0, 0, 255);
    Magick::Geometry dimensions(Magick::Geometry(ORIGINAL_TILE_WIDTH * f_columns, ORIGINAL_TILE_HEIGHT * f_rows, 0, 0));

    std::unique_ptr<Magick::Image> board;
    Magick::PixelPacket * data(nullptr);
    Magick::PixelPacket * data_end(nullptr);
    if(save_image)
    {
        board = std::make_unique<Magick::Image>(dimensions, black);
        data = board->setPixels(0, 0, ORIGINAL_TILE_WIDTH * f_columns, ORIGINAL_TILE_HEIGHT * f_rows);
        data_end = data + ORIGINAL_TILE_WIDTH * f_columns * 3 * ORIGINAL_TILE_HEIGHT * f_rows;
    }

    // TODO: to make it much more practical (easier to read) we would have
    //       to number the tiles according to what they represent instead
    //       of what we have now
    //
    for(int r(0); r < f_rows; ++r)
    {
        for(int c(0); c < f_columns; ++c)
        {
            if(c != 0)
            {
                out << ", ";
            }
            int tile(f_board[board_position(c, r)]);
            out << std::setw(2) << tile;

            if(data != nullptr)
            {
                Magick::PixelPacket * o(data
                            + c * ORIGINAL_TILE_WIDTH
                            + r * ORIGINAL_TILE_HEIGHT * f_columns * ORIGINAL_TILE_WIDTH);
                if(tile > 0)
                {
                    std::uint8_t const * const i(f_tiles[tile - 1] + 1);
                    for(int y(0); y < ORIGINAL_TILE_HEIGHT; ++y)
                    {
                        Magick::PixelPacket * pixel(o + y * f_columns * ORIGINAL_TILE_WIDTH);
                        if(pixel >= data_end)
                        {
                            throw std::overflow_error("pixel pointer out of bounds.");
                        }
                        for(int x(0); x < ORIGINAL_TILE_WIDTH; ++x)
                        {
                            Magick::Quantum component(i[y * ORIGINAL_TILE_WIDTH * 3 + x * 3 + 0]);
                            pixel[x].red = component * 256 + component;
                            component = i[y * ORIGINAL_TILE_WIDTH * 3 + x * 3 + 1];
                            pixel[x].green = component * 256 + component;
                            component = i[y * ORIGINAL_TILE_WIDTH * 3 + x * 3 + 2];
                            pixel[x].blue = component * 256 + component;
                            pixel[x].opacity = 0;
                        }
                    }
                }
                //else keep black if removed
            }
        }
        out << "\n";
    }

    if(board != nullptr)
    {
        std::string filename(f_save_board + ".png");
        std::string::size_type const pos(f_save_board.find('.'));
        if(pos != std::string::npos)
        {
            filename = f_save_board.substr(0, pos) + ".png";
        }
        board->write(filename);
    }

    return 0;
}


bool resolver::next_position(position_t & p)
{
    switch(p.f_direction)
    {
    case DIRECTION_NONE:
        if(p.f_row >= f_rows)
        {
            return false;
        }
        ++p.f_column;
        if(p.f_column >= f_columns)
        {
            ++p.f_row;
            if(p.f_row >= f_rows)
            {
                return false;
            }
            p.f_column = 0;
        }
        return true;

    case DIRECTION_LEFT:
        if(p.f_column <= 0)
        {
            return false;
        }
        --p.f_column;
        return true;

    case DIRECTION_UP:
        if(p.f_row <= 0)
        {
            return false;
        }
        --p.f_row;
        return true;

    case DIRECTION_RIGHT:
        if(p.f_column + 1 >= f_columns)
        {
            return false;
        }
        ++p.f_column;
        return true;

    case DIRECTION_DOWN:
        if(p.f_row + 1 >= f_rows)
        {
            return false;
        }
        ++p.f_row;
        return true;

    }

    throw std::logic_error("invalid direction in next_direction().");
}


resolver::position_t::list_t resolver::find_matches(position_t const & p1)
{
    position_t::list_t result;

    int const tile(f_board[board_position(p1)]);
    for(int8_t r(0); r < f_rows; ++r)
    {
        for(int8_t c(0); c < f_columns; ++c)
        {
            if(f_board[board_position(c, r)] == tile)
            {
                position_t const p2{
                    .f_column = c,
                    .f_row = r,
                };
                if(!p1.same_position(p2))
                {
                    result.push_back(p2);
                }
            }
        }
    }

    if(result.size() > 3)
    {
        throw std::logic_error(
              "found more matching tile than possible ("
            + std::to_string(result.size())
            + " x "
            + std::to_string(tile)
            + ").");
    }

    return result;
}


int resolver::resolve_board()
{
    // to resolve the board, we want to systematically check each tile and
    // move forward and backward through our f_steps tree
    //

    f_first_step = std::make_shared<step_t>();
    step_t::pointer_t current_step(f_first_step);

int move(1);

    position_t p1{
        .f_column = 0,
        .f_row = 0,
    };
    bool reset(false);
    bool empty(true);
    for(;;)
    {
go_to_next_position:
        step_t::vector_t const matches(match_tiles(p1));
        if(matches.size() > 0)
        {
            board_t save(f_board);

            int const tile(f_board[board_position(p1)]);
            remove_tile(p1);

            position_t p2(matches[0]->f_p2);
            if(p2.f_column == p1.f_column
            && p2.f_row < p1.f_row)
            {
                // special case were p2 is in the same column as p1
                // and p1 was under p2, in that case, p2 moved down
                // by one row!
                //
                ++p2.f_row;
                if(p2.f_row >= f_rows)
                {
                    throw std::logic_error("somehow the p2.f_row + 1 went over the limit.");
                }
            }
            remove_tile(p2);

            auto const it(std::find_if(
                      f_board_resolved.begin()
                    , f_board_resolved.end()
                    , [this](board_pointer_t b) {
                        return this->f_board == *b;
                    }));
            if(it == f_board_resolved.end())
            {
std::cerr << "---------- " << move << ". removing p1 ("
<< static_cast<int>(p1.f_column) << ", " << static_cast<int>(p1.f_row) << ") and p2 ("
<< static_cast<int>(matches[0]->f_p2.f_column) << ", " << static_cast<int>(matches[0]->f_p2.f_row) << ").\n";

                reset = true;
                empty = false;

#ifdef DEBUG_WITH_STACK
                f_board_stack.push_back(std::make_shared<board_t>(save));
#endif

                std::for_each(matches.begin(), matches.end(), [&current_step, tile](step_t::pointer_t s) {
                        s->f_parent = current_step;
                        s->f_tile = tile;
                    });
                current_step->f_children.insert(current_step->f_children.end(), matches.begin(), matches.end());

                current_step = matches[0];

                f_board_resolved.push_back(std::make_shared<board_t>(f_board));

{
std::stringstream filename;
filename << "boards/" << std::setw(8) << std::setfill('0') << move << "-board.txt";
f_save_board = filename.str();
std::cerr << "--- save extra board [" << f_save_board << "]\n";
save_board();
++move;
}

                // reset position for next search
                //
                p1.f_column = 0;
                p1.f_row = 0;
                while(f_board[board_position(p1)] == 0)
                {
                    if(!next_position(p1))
                    {
                        //throw std::logic_error("we just restore a tile so at least there is that one on the screen, right?");
                        std::cout << "info: board resolved.\n";
                        return 0;
                    }
                }
                continue;
            }

            // that position was already checked, skip it
            //
            f_board = save;
        }
        do
        {
            if(!next_position(p1))
            {
                if(!reset)
                {
//                    if(empty)
//                    {
//std::cerr << "----------- reached the end for the second time and no tiles, we're done\n";
//                        return 0;
//                    }
std::cerr << "----------- reached the end and !empty so this is a deadend (not a solution); try next branch in tree...\n";

                    // we may have to go up our tree multiple times in a row
                    //
                    do
                    {
std::cerr << "--- current step"
#ifdef DEBUG_WITH_STACK
<< "[" << f_board_stack.size() << "]"
#endif
<< ": p1 ("
<< static_cast<int>(current_step->f_p1.f_column) << ", " << static_cast<int>(current_step->f_p1.f_row) << ") and p2 ("
<< static_cast<int>(current_step->f_p2.f_column) << ", " << static_cast<int>(current_step->f_p2.f_row) << ") -> "
<< static_cast<int>(current_step->f_tile) << ".\n";

                        p1 = current_step->f_p1;
                        position_t p2(current_step->f_p2);
                        if(p2.f_column == p1.f_column
                        && p2.f_row < p1.f_row)
                        {
                            // special case were p2 is in the same column as p1
                            // and p1 was under p2, in that case, p2 moved down
                            // by one row so move it back up
                            //
                            ++p2.f_row;
                        }
                        restore_tile(p2, current_step->f_tile);
                        restore_tile(p1, current_step->f_tile);

{
std::stringstream filename;
filename << "boards/" << std::setw(8) << std::setfill('0') << move << "-restore.txt";
f_save_board = filename.str();
std::cerr << "--- restored: save extra board [" << f_save_board << "] ("
<< current_step->f_children.size() << ")\n";
save_board();
++move;
}

#ifdef DEBUG_WITH_STACK
                        if(f_board_stack.empty())
                        {
                            throw std::logic_error("trying to restore when stack is empty.");
                        }
                        if(*f_board_stack.back() != f_board)
                        {
{
f_board = *f_board_stack.back();
std::stringstream filename;
filename << "boards/" << std::setw(8) << std::setfill('0') << (move - 1) << "-stack.txt";
f_save_board = filename.str();
std::cerr << "--- stack: save extra board [" << f_save_board << "] ("
<< current_step->f_children.size() << ")\n";
save_board();
}
                            throw std::logic_error("restore and stack are not equal.");
                        }
                        f_board_stack.pop_back();
#endif

                        if(!current_step->f_children.empty())
                        {
                            throw std::logic_error("current_step has children when restoring?");
                        }
                        step_t::pointer_t parent(current_step->f_parent);
                        if(parent->f_children.empty())
                        {
                            throw std::logic_error("parent has no children, not even current_step?");
                        }
                        if(parent->f_children[0] != current_step)
                        {
                            throw std::logic_error("parent first child is not current_step?");
                        }
                        parent->f_children.erase(parent->f_children.begin());
                        while(!parent->f_children.empty())
                        {
                            current_step = parent->f_children[0];

////////////////////////////////////////////////////////////////////////////////////////

                            // apply that next match (this happens with one
                            // tile matches 2 or 3 of the other tiles)
                            //
                            board_t save(f_board);

                            p1 = current_step->f_p1;
                            remove_tile(p1);

                            p2 = current_step->f_p2;
                            if(p2.f_column == p1.f_column
                            && p2.f_row < p1.f_row)
                            {
                                // special case were p2 is in the same column as p1
                                // and p1 was under p2, in that case, p2 moved down
                                // by one row!
                                //
                                ++p2.f_row;
                                if(p2.f_row >= f_rows)
                                {
                                    throw std::logic_error("somehow the p2.f_row + 1 went over the limit.");
                                }
                            }
                            remove_tile(p2);

                            auto const it(std::find_if(
                                  f_board_resolved.begin()
                                , f_board_resolved.end()
                                , [this](board_pointer_t b) {
                                    return this->f_board == *b;
                                }));
                            if(it == f_board_resolved.end())
                            {
std::cerr << "---------- " << move << ". removing case II -- p1 ("
<< static_cast<int>(p1.f_column) << ", " << static_cast<int>(p1.f_row) << ") and p2 ("
<< static_cast<int>(p2.f_column) << ", " << static_cast<int>(p2.f_row) << ").\n";

                                reset = true;
                                empty = false;

#ifdef DEBUG_WITH_STACK
                                f_board_stack.push_back(std::make_shared<board_t>(save));
#endif

                                f_board_resolved.push_back(std::make_shared<board_t>(f_board));

{
std::stringstream filename;
filename << "boards/" << std::setw(8) << std::setfill('0') << move << "-board.txt";
f_save_board = filename.str();
std::cerr << "--- save extra board -- case II [" << f_save_board << "]\n";
save_board();
++move;
}

                                // reset position for next search
                                //
                                p1.f_column = 0;
                                p1.f_row = 0;
                                while(f_board[board_position(p1)] == 0)
                                {
                                    if(!next_position(p1))
                                    {
                                        //throw std::logic_error("we just restore a tile so at least there is that one on the screen, right?");
                                        std::cout << "info: board resolved.\n";
                                        return 0;
                                    }
                                }
                                goto go_to_next_position;
                            }

                            // that position was already checked, skip it
                            //
                            f_board = save;

////////////////////////////////////////////////////////////////////////////////////////

                            parent->f_children.erase(parent->f_children.begin());
                        }
                        current_step = parent;
                        if(current_step->f_parent == nullptr)
                        {
std::cerr << "----------- reached the end for the second time and no tiles, we're done\n";
                            return 0;
                        }
                    }
                    while(!next_position(p1));
std::cerr << "----------- loop again -- after a step back + next_position("
<< static_cast<int>(p1.f_column) << ", " << static_cast<int>(p1.f_row) << ")\n";
                }
                else
                {
std::cerr << "----------- reached the end...\n";
                    reset = false;
                    empty = true;
                    p1.f_column = 0;
                    p1.f_row = 0;
                }
            }
//std::cerr << "info: match_tiles(" << static_cast<int>(p1.f_column) << ", " << static_cast<int>(p1.f_row) << ") returned tile " << static_cast<int>(f_board[board_position(p1)]) << " at that position\n";
        }
        while(f_board[board_position(p1)] == 0);
//std::cerr << "----------- loop again...(" << static_cast<int>(p1.f_column) << ", " << static_cast<int>(p1.f_row) << ")\n";
    }

    return 1;
}


bool resolver::is_tile(
      Magick::PixelPacket const * ptr
    , int x
    , int y
    , int width
    , int height) const
{
    static_cast<int>(height);

    int bad(0);
    for(int h(0); h < 8; ++h)
    {
        for(int w(0); w < 8; ++w)
        {
            Magick::PixelPacket const * p(ptr + (x + w) + (y + h) * width); // TODO: use increments
            if(h == 0)
            {
                // the first line must be 100% white
                //
                if(p->red != 0xFFFF
                || p->green != 0xFFFF
                || p->blue != 0xFFFF)
                {
                    return false;
                }
            }
            else
            {
                if(p->red < 0xF8F8
                || p->green < 0xF8F8
                || p->blue < 0xF8F8)
                {
                    ++bad;
                    if(bad > 6)
                    {
                        return false;
                    }
                }
            }
        }
    }
    return true;
}



int resolver::find_right_edge(
      Magick::PixelPacket const * ptr
    , int x
    , int y
    , int width
    , int height) const
{
    static_cast<void>(height);

    // our sample uses 100 pixels in width
    //
    for(int w(0); w < 200; ++w)
    {
        if(w + x >= width)
        {
            return -1;
        }

        Magick::PixelPacket const * p(ptr + x + w + y * width); // TODO: use increments
        if(p->red < 0xB0B0
        && p->green < 0xB0B0
        && p->blue < 0xB0B0)
        {
            return x + w;
        }
    }
    return -1;
}


int resolver::find_bottom_edge(
      Magick::PixelPacket const * ptr
    , int x
    , int y
    , int width
    , int height) const
{
    // our sample uses about 130 pixels in height
    // also the number may be in the way at the top so we skip a few lines
    // immediately to avoid that issue
    //
    for(int h(30); h < 200; ++h)
    {
        if(y + h >= height)
        {
std::cerr << "error: image height reached?\n";
            return -1;
        }

        Magick::PixelPacket const * p(ptr + x + (y + h) * width); // TODO: use increments
//std::cerr << "find bottom: pixel at (" << x << ", " << y + h << ") -> ("
//<< p->red << ", " << p->green << ", " << p->blue << ")\n";
        if(p->red < 0xB0B0
        && p->green < 0xB0B0
        && p->blue < 0xB0B0)
        {
            std::uint8_t const r(p->red >> 8);
            std::uint8_t const g(p->green >> 8);
            std::uint8_t const b(p->blue >> 8);
            if(labs(r - g) < 4
            && labs(r - b) < 4
            && labs(g - b) < 4)
            {
                return y + h;
            }
        }
    }
std::cerr << "error: 200 pixels not enough?\n";
    return -1;
}


int resolver::find_tile(
      Magick::PixelPacket const * ptr
    , int x
    , int y
    , int width
    , int height
    , int tile_width
    , int tile_height)
{
    static_cast<void>(height);

    for(auto const & buffer : f_tiles)
    {
        bool found(true);
        for(int h(0); h < tile_height && found; ++h)
        {
            for(int w(0); w < tile_width; ++w)
            {
                Magick::PixelPacket const * p(ptr + x + w + (y + h) * width);
                if(labs(buffer[(w + h * tile_width) * 3 + 1] - (p->red >> 8)) > 8
                || labs(buffer[(w + h * tile_width) * 3 + 2] - (p->green >> 8)) > 8
                || labs(buffer[(w + h * tile_width) * 3 + 3] - (p->blue >> 8)) > 8)
                {
                    found = false;
                    break;
                }
            }
        }
        if(found)
        {
            return buffer[0];
        }
    }
    return 0;
}


void resolver::add_tile(
      Magick::PixelPacket const * ptr
    , int x
    , int y
    , int width
    , int height
    , int tile_width
    , int tile_height)
{
    static_cast<void>(height);

    // x3 for red, green, blue
    std::uint8_t * buffer = new std::uint8_t[1 + tile_width * tile_height * 3];
    buffer[0] = static_cast<std::uint8_t>(f_tiles.size()) + 1;
    for(int h(0); h < tile_height; ++h)
    {
        for(int w(0); w < tile_width; ++w)
        {
            Magick::PixelPacket const * p(ptr + x + w + (y + h) * width);
            buffer[(w + h * tile_width) * 3 + 1] = p->red >> 8;
            buffer[(w + h * tile_width) * 3 + 2] = p->green >> 8;
            buffer[(w + h * tile_width) * 3 + 3] = p->blue >> 8;
        }
    }
    f_tiles.push_back(buffer);
}


void resolver::output_tile(
      std::ofstream & generate_out
    , Magick::PixelPacket const * ptr
    , int x
    , int y
    , int width
    , int height
    , int tile_width
    , int tile_height)
{
    static_cast<void>(height);

    int column = 0;
    auto output_hex = [&generate_out, &column](int value)
    {
        generate_out << value;
        ++column;
        if(column >= 16)
        {
            column = 0;
            generate_out << "\n";
        }
    };

    //generate_out << '{';
    generate_out << "/* " << f_tiles.size() << " */ ";
    output_hex(f_tiles.size());
    for(int h(0); h < tile_height; ++h)
    {
        for(int w(0); w < tile_width; ++w)
        {
            // TODO: add new lines every X hexadecimal number
            //
            Magick::PixelPacket const * p(ptr + x + w + (y + h) * width);
            generate_out << ',';
            output_hex(p->red >> 8);
            generate_out << ',';
            output_hex(p->green >> 8);
            generate_out << ',';
            output_hex(p->blue >> 8);
        }
    }
    //generate_out << "},\n";
    generate_out << ",\n";
}


resolver::step_t::vector_t resolver::match_tiles(position_t const & p1)
{
    // look at still existing matches and try to draw a line in between
    //
    int const tile1(f_board[board_position(p1)]);
    if(tile1 == 0)
    {
        throw std::logic_error("match_tiles() cannot have p1 over an empty location.");
    }
    position_t::list_t matches(find_matches(p1));
    if(matches.empty())
    {
        throw std::logic_error(
              "match_tiles() cannot find matches for p1 (we did not have an even number of p1 tiles ("
            + std::to_string(tile1)
            + ")?).");
    }

    step_t::vector_t result;

    for(auto const & p2 : matches)
    {
        if(p1.f_column == p2.f_column
        && p1.f_row == p2.f_row)
        {
            // skip self
            //
            continue;
        }
        int const tile2(f_board[board_position(p2)]);
        if(tile2 == 0)
        {
            throw std::logic_error("got a match (p2) pointing at a 0 tile?");
        }

        // try drawing a line between p1 and p2
        //
        if(find_connection(p1, p2))
        {
            step_t::pointer_t step(std::make_shared<step_t>());
            step->f_p1 = p1,
            step->f_p2 = p2,
            result.push_back(step);
        }
    }

    return result;
}


bool resolver::find_connection(position_t p1, position_t p2)
{
    // here are the possible lines:
    //
    //   p2 <- p1   p2 -> p1  p1  p2
    //                        |   ^ 
    //                        v   | 
    //                        p2  p1
    //
    //   p2 <-+   +-> p2   p1 -+    +- p1
    //        |   |            |    |
    //        p1  p1           v    v
    //                         p2   p2
    //
    //   p2 <-+          +-> p2   p1 -+            +- p1
    //        |          |            |            |
    //        +- p1  p1 -+            +-> p2  p2 <-+
    //
    //   +-> p2  p2 <-+   +---+   +---+    p1  p2  p2  p1
    //   |            |   |   |   |   |    |   ^   ^   |
    //   +-- p1  p1 --+   |   v   v   |    |   |   |   |
    //                    p1  p2  p2  p1   +---+   +---+

    typedef std::function<bool(position_t p1, position_t p2)> connect_t;

    static connect_t const connections[17] = {
        std::bind(&resolver::connect_left, this, std::placeholders::_1, std::placeholders::_2),
        std::bind(&resolver::connect_right, this, std::placeholders::_1, std::placeholders::_2),
        std::bind(&resolver::connect_down, this, std::placeholders::_1, std::placeholders::_2),
        std::bind(&resolver::connect_up, this, std::placeholders::_1, std::placeholders::_2),

        std::bind(&resolver::connect_up_left, this, std::placeholders::_1, std::placeholders::_2),
        std::bind(&resolver::connect_up_right, this, std::placeholders::_1, std::placeholders::_2),
        std::bind(&resolver::connect_right_down, this, std::placeholders::_1, std::placeholders::_2),
        std::bind(&resolver::connect_left_down, this, std::placeholders::_1, std::placeholders::_2),

        std::bind(&resolver::connect_left_up_left, this, std::placeholders::_1, std::placeholders::_2),
        std::bind(&resolver::connect_right_up_right, this, std::placeholders::_1, std::placeholders::_2),
        std::bind(&resolver::connect_right_down_right, this, std::placeholders::_1, std::placeholders::_2),
        std::bind(&resolver::connect_left_down_left, this, std::placeholders::_1, std::placeholders::_2),

        std::bind(&resolver::connect_left_up_right, this, std::placeholders::_1, std::placeholders::_2),
        std::bind(&resolver::connect_right_up_left, this, std::placeholders::_1, std::placeholders::_2),
        std::bind(&resolver::connect_up_right_down, this, std::placeholders::_1, std::placeholders::_2),
        std::bind(&resolver::connect_up_left_down, this, std::placeholders::_1, std::placeholders::_2),
        // this last one is very simple and does not require separate left and right cases
        std::bind(&resolver::connect_down_left_or_right_up, this, std::placeholders::_1, std::placeholders::_2),
    };

    for(auto const & f : connections)
    {
        if(f(p1, p2))
        {
            return true;
        }
    }

    return false;
}


bool resolver::connect_left(position_t p1, position_t p2)
{
    if(p1.f_column < p2.f_column)
    {
        return false;
    }
    p1.f_direction = DIRECTION_LEFT;
    for(;;)
    {
        if(!next_position(p1))
        {
            return false;
        }
        if(f_board[board_position(p1)] != 0)
        {
            return p1.same_position(p2);
        }
    }
}


bool resolver::connect_right(position_t p1, position_t p2)
{
    if(p1.f_column > p2.f_column)
    {
        return false;
    }
    p1.f_direction = DIRECTION_RIGHT;
    for(;;)
    {
        if(!next_position(p1))
        {
            return false;
        }
        if(f_board[board_position(p1)] != 0)
        {
            return p1.same_position(p2);
        }
    }
}


bool resolver::connect_down(position_t p1, position_t p2)
{
    return p1.f_column == p2.f_column
        && p1.f_row + 1 == p2.f_row;
}


bool resolver::connect_up(position_t p1, position_t p2)
{
    return p1.f_column == p2.f_column
        && p1.f_row - 1 == p2.f_row;
}


bool resolver::connect_up_left(position_t p1, position_t p2)
{
    if(p1.f_column < p2.f_column
    || p1.f_row < p2.f_row)
    {
        return false;
    }

    for(;;)
    {
        p1.f_direction = DIRECTION_UP;
        if(!next_position(p1))
        {
            return false;
        }
        int tile(f_board[board_position(p1)]);
        if(tile != 0)
        {
            return false;
        }
        if(connect_left(p1, p2))
        {
            return true;
        }
    }
}


bool resolver::connect_up_right(position_t p1, position_t p2)
{
    if(p1.f_column > p2.f_column
    || p1.f_row < p2.f_row)
    {
        return false;
    }

    for(;;)
    {
        p1.f_direction = DIRECTION_UP;
        if(!next_position(p1))
        {
            return false;
        }
        int tile(f_board[board_position(p1)]);
        if(tile != 0)
        {
            return false;
        }
        if(connect_right(p1, p2))
        {
            return true;
        }
    }
}


bool resolver::connect_right_down(position_t p1, position_t p2)
{
    if(p1.f_column > p2.f_column
    || p1.f_row > p2.f_row)
    {
        return false;
    }

    for(;;)
    {
        p1.f_direction = DIRECTION_RIGHT;
        if(!next_position(p1))
        {
            return false;
        }
        int tile(f_board[board_position(p1)]);
        if(tile != 0)
        {
            return false;
        }
        position_t d(p1);
        for(;;)
        {
            d.f_direction = DIRECTION_DOWN;
            if(!next_position(d))
            {
                break;
            }
            if(d.same_position(p2))
            {
                return true;
            }
            tile = f_board[board_position(d)];
            if(tile != 0)
            {
                break;
            }
        }
    }
}


bool resolver::connect_left_down(position_t p1, position_t p2)
{
    if(p1.f_column < p2.f_column
    || p1.f_row > p2.f_row)
    {
        return false;
    }

    for(;;)
    {
        p1.f_direction = DIRECTION_LEFT;
        if(!next_position(p1))
        {
            return false;
        }
        int tile(f_board[board_position(p1)]);
        if(tile != 0)
        {
            return false;
        }
        position_t d(p1);
        for(;;)
        {
            d.f_direction = DIRECTION_DOWN;
            if(!next_position(d))
            {
                break;
            }
            if(d.same_position(p2))
            {
                return true;
            }
            tile = f_board[board_position(d)];
            if(tile != 0)
            {
                break;
            }
        }
    }
}


bool resolver::connect_left_up_left(position_t p1, position_t p2)
{
    if(p1.f_column < p2.f_column
    || p1.f_row < p2.f_row)
    {
        return false;
    }

    // TODO: there is a better way to do this search which is to search
    //       check that the left is 0, go up once, find the edge on the
    //       left and then "follow the edge"

    //for(;;)
    {
        p1.f_direction = DIRECTION_LEFT;
        if(!next_position(p1))
        {
            return false;
        }
        if(f_board[board_position(p1)] != 0)
        {
            return false;
        }
        position_t u(p1);
        u.f_direction = DIRECTION_UP;
        for(;;)
        {
            if(!next_position(u))
            {
                break;
            }
            if(f_board[board_position(u)] != 0)
            {
                throw std::logic_error("up in left_up_left() is not zero?");
            }
            position_t l(u);
            l.f_direction = DIRECTION_LEFT;
            for(;;)
            {
                if(!next_position(l))
                {
                    break;
                }
                if(f_board[board_position(l)] != 0)
                {
                    if(l.same_position(p2))
                    {
                        return true;
                    }
                    break;
                }
            }
        }
    }

    return false;
}


bool resolver::connect_right_up_right(position_t p1, position_t p2)
{
    if(p1.f_column > p2.f_column
    || p1.f_row < p2.f_row)
    {
        return false;
    }

    // TODO: there is a better way to do this search which is to search
    //       check that the right is 0, go up once, find the edge on the
    //       right and then "follow the edge"

    //for(;;)
    {
        p1.f_direction = DIRECTION_RIGHT;
        if(!next_position(p1))
        {
            return false;
        }
        if(f_board[board_position(p1)] != 0)
        {
            return false;
        }
        position_t u(p1);
        u.f_direction = DIRECTION_UP;
        for(;;)
        {
            if(!next_position(u))
            {
                break;
            }
            if(f_board[board_position(u)] != 0)
            {
                throw std::logic_error("up in right_up_right() is not zero?");
            }
            position_t r(u);
            r.f_direction = DIRECTION_RIGHT;
            for(;;)
            {
                if(!next_position(r))
                {
                    break;
                }
                if(f_board[board_position(r)] != 0)
                {
                    if(r.same_position(p2))
                    {
                        return true;
                    }
                    break;
                }
            }
        }
    }

    return false;
}


bool resolver::connect_right_down_right(position_t p1, position_t p2)
{
    if(p1.f_column > p2.f_column
    || p1.f_row > p2.f_row)
    {
        return false;
    }

    for(;;)
    {
        p1.f_direction = DIRECTION_RIGHT;
        if(!next_position(p1))
        {
            return false;
        }
        if(f_board[board_position(p1)] != 0)
        {
            return false;
        }
        position_t u(p1);
        u.f_direction = DIRECTION_DOWN;
        for(;;)
        {
            if(!next_position(u))
            {
                break;
            }
            if(f_board[board_position(u)] != 0)
            {
                break;
            }
            position_t r(u);
            r.f_direction = DIRECTION_RIGHT;
            for(;;)
            {
                if(!next_position(r))
                {
                    break;
                }
                if(f_board[board_position(r)] != 0)
                {
                    if(r.same_position(p2))
                    {
                        return true;
                    }
                    break;
                }
            }
        }
    }
}


bool resolver::connect_left_down_left(position_t p1, position_t p2)
{
    if(p1.f_column < p2.f_column
    || p1.f_row > p2.f_row)
    {
        return false;
    }

    for(;;)
    {
        p1.f_direction = DIRECTION_LEFT;
        if(!next_position(p1))
        {
            return false;
        }
        if(f_board[board_position(p1)] != 0)
        {
            return false;
        }
        position_t u(p1);
        u.f_direction = DIRECTION_DOWN;
        for(;;)
        {
            if(!next_position(u))
            {
                break;
            }
            if(f_board[board_position(u)] != 0)
            {
                break;
            }
            position_t l(u);
            l.f_direction = DIRECTION_LEFT;
            for(;;)
            {
                if(!next_position(l))
                {
                    break;
                }
                if(f_board[board_position(l)] != 0)
                {
                    if(l.same_position(p2))
                    {
                        return true;
                    }
                    break;
                }
            }
        }
    }
}


bool resolver::connect_left_up_right(position_t p1, position_t p2)
{
    if(p1.f_column > p2.f_column
    || p1.f_row < p2.f_row)
    {
        return false;
    }

    for(;;)
    {
        p1.f_direction = DIRECTION_LEFT;
        if(!next_position(p1))
        {
            return false;
        }
        if(f_board[board_position(p1)] != 0)
        {
            return false;
        }
        position_t u(p1);
        u.f_direction = DIRECTION_UP;
        for(;;)
        {
            if(!next_position(u))
            {
                break;
            }
            if(f_board[board_position(u)] != 0)
            {
                throw std::logic_error("up in left_up_left() is not zero?");
            }
            position_t r(u);
            r.f_direction = DIRECTION_RIGHT;
            for(;;)
            {
                if(!next_position(r))
                {
                    break;
                }
                if(f_board[board_position(r)] != 0)
                {
                    if(r.same_position(p2))
                    {
                        return true;
                    }
                    break;
                }
            }
        }
    }
}


bool resolver::connect_right_up_left(position_t p1, position_t p2)
{
    if(p1.f_column > p2.f_column
    || p1.f_row < p2.f_row)
    {
        return false;
    }

    for(;;)
    {
        p1.f_direction = DIRECTION_RIGHT;
        if(!next_position(p1))
        {
            return false;
        }
        if(f_board[board_position(p1)] != 0)
        {
            return false;
        }
        position_t u(p1);
        u.f_direction = DIRECTION_UP;
        for(;;)
        {
            if(!next_position(u))
            {
                break;
            }
            if(f_board[board_position(u)] != 0)
            {
                throw std::logic_error("up in connect_right_up_left() is not zero?");
            }
            position_t r(u);
            r.f_direction = DIRECTION_LEFT;
            for(;;)
            {
                if(!next_position(r))
                {
                    break;
                }
                if(f_board[board_position(r)] != 0)
                {
                    if(r.same_position(p2))
                    {
                        return true;
                    }
                    break;
                }
            }
        }
    }
}


bool resolver::connect_up_right_down(position_t p1, position_t p2)
{
    if(p1.f_column >= p2.f_column)
    {
        return false;
    }

    //for(;;)
    {
        p1.f_direction = DIRECTION_UP;
        if(next_position(p1))
        {
            if(f_board[board_position(p1)] != 0)
            {
                return false;
            }
        }
        // here there is no need to test going right, we can always jump
        // straight to the p2 column
        //
        position_t d(p1);
        d.f_direction = DIRECTION_DOWN;
        d.f_row = 0;
        d.f_column = p2.f_column;
        for(;;)
        {
            if(f_board[board_position(d)] != 0)
            {
                return d.same_position(p2);
            }
            if(!next_position(d))
            {
                break;
            }
        }
    }

    return false;
}


bool resolver::connect_up_left_down(position_t p1, position_t p2)
{
    if(p1.f_column <= p2.f_column)
    {
        return false;
    }

    //for(;;)
    {
        p1.f_direction = DIRECTION_UP;
        if(next_position(p1))
        {
            if(f_board[board_position(p1)] != 0)
            {
                return false;
            }
        }
        // here there is no need to test going left, we can always jump
        // straight to the p2 column
        //
        position_t d(p1);
        d.f_direction = DIRECTION_DOWN;
        d.f_row = 0;
        d.f_column = p2.f_column;
        for(;;)
        {
            if(f_board[board_position(d)] != 0)
            {
                return d.same_position(p2);
            }
            if(!next_position(d))
            {
                break;
            }
        }
    }

    return false;
}


bool resolver::connect_down_left_or_right_up(position_t p1, position_t p2)
{
    return p1.f_row == f_rows - 1
        && p2.f_row == f_rows - 1;
}


void resolver::remove_tile(position_t const & p)
{
    int r(p.f_row);
    while(r > 0)
    {
        tile_t const item(f_board[board_position(p.f_column, r - 1)]);
        f_board[board_position(p.f_column, r)] = item;
        if(item == 0)
        {
            return;
        }
        --r;
    }
    f_board[board_position(p.f_column, r)] = 0;
}


void resolver::restore_tile(position_t const & p, tile_t tile)
{
    if(f_board[board_position(p.f_column, 0)] != 0)
    {
        throw std::logic_error(
              "trying to restore in a full column ("
            + std::to_string(p.f_column)
            + ").");
    }

    for(int r(1); r <= p.f_row; ++r)
    {
        f_board[board_position(p.f_column, r - 1)] = f_board[board_position(p.f_column, r)];
    }
    f_board[board_position(p.f_column, p.f_row)] = tile;
}


int main(int argc, char * argv[])
{
    resolver rr;
    int const r(rr.parse_args(argc, argv));
    if(r != 0)
    {
        exit(r);
    }

    return rr.run();
}


// vim: ts=4 sw=4 et
