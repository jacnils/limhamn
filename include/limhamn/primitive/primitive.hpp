#pragma once

#include <cairo/cairo.h>
#include <pango/pango.h>
#include <pango/pangocairo.h>
#include <string>
#include <stdexcept>
#include <cmath>
#include <algorithm>
#include <sstream>

#define LIMHAMN_PRIMITIVE

#ifndef LIMHAMN_PRIMITIVE_CANVAS
#define LIMHAMN_PRIMITIVE_CANVAS 1
#endif

#if LIMHAMN_PRIMITIVE_X11
#include <X11/Xlib.h>
#include <cairo/cairo-xlib.h>
#endif

/**
 * @brief Namespace for the primitive drawing library.
 */
namespace limhamn::primitive {
    /**
     * @brief Class for managing fonts in the drawing context.
     * @note Quite primitive, for internal use by the draw_manager class.
     */
    class font_manager {
        unsigned int h{};
        PangoLayout* layout{};
        bool active{false};
    public:
        explicit font_manager() = default;
        /**
         * @brief Initializes the font manager with a specified font.
         * @param font The name of the font to be used. (e.g "Sans 12")
         */
        void init_font(const std::string& font);
        /**
         * @brief Estimates the length of a given text string.
         * @pre The font manager must be initialized with a font.
         * @param text The text string to estimate the length of.
         * @param length The length of the text string. If -1, the length is calculated automatically.
         * @param markup If true, markup is applied to the text.
         * @return A pair of integers representing the estimated width and height of the text.
         */
        [[nodiscard]] std::pair<int,int> estimate_length(const std::string& text, int length = -1, bool markup = true) const;
        /**
         * @brief Gets the internal PangoLayout object as a reference.
         * @pre The font manager must be initialized with a font.
         * @return A reference to the PangoLayout object.
         */
        [[nodiscard]] PangoLayout& get_layout();
        /**
         * @brief Initializes the font manager with a specified font.
         * @param font The name of the font to be used. (e.g "Sans 12")
         */
        explicit font_manager(const std::string& font);
        /**
         * @brief Checks if the font manager is active.
         * @return True if the font manager is active, false otherwise.
         */
        [[nodiscard]] bool is_active() const;
        /**
        * @brief Gets the height of the font.
        * @return The height of the font.
        */
        [[nodiscard]] unsigned int get_height() const;
        ~font_manager();

        friend class draw_manager;
    };

    /**
     * @brief Enumeration for different drawing protocols.
     * @note X11 will use Xlib and Cairo for drawing.
     * @note Canvas will use Cairo directly with an image buffer (void*).
     */
    enum class protocol : int {
        x11 = 0,
        canvas = 1,
        unknown = -1
    };

#if LIMHAMN_PRIMITIVE_X11
    /**
     * @brief Structure for managing X11 window properties.
     * @note This structure is used for X11 drawing.
     */
    struct x11_window {
        Display* dpy{};
        int screen{};
        Window root{};
        Visual* visual{};
        unsigned int depth{};
        Colormap cmap{};
        Drawable drawable{};
        GC gc{};
    };
#endif

#if LIMHAMN_PRIMITIVE_CANVAS
    /**
     * @brief Structure for managing canvas window properties.
     * @note This structure is used for canvas drawing.
     */
    struct canvas_window {
        void* data{};
    };
#endif
    /**
     * @brief Structure for managing image properties.
     * @note This structure is used for image drawing.
     */
    struct image_manager {
        uint8_t* data{};
        cairo_surface_t* img_surface{};
    };
    /**
     * @brief Structure for managing drawing properties.
     * @note This structure is used for setting colors and styles.
     */
    struct draw_properties {
        std::string foreground{};
        std::string background{};
        int foreground_alpha{255};
        int background_alpha{255};
        bool invert{false};
        bool filled{true};
    };
    /**
     * @brief Structure for managing drawing shapes.
     * @note This structure is used for setting shapes and styles.
     */
    struct draw_shape_properties {
        std::string prev{};
        std::string next{};
        int prev_alpha{255};
        int next_alpha{255};
    };
    /**
     * @brief Structure for managing drawing positions.
     * @note This structure is used for setting positions and sizes.
     */
    struct draw_position {
        int x{};
        int y{};
        int w{};
        int h{};
    };
    /**
     * @brief Structure for managing drawing coordinates.
     * @note This structure is used for setting coordinates.
     */
    struct draw_coords {
        int x{};
        int y{};
    };
    /**
     * @brief Structure for managing drawing sizes.
     * @note This structure is used for setting sizes.
     */
    struct draw_size {
        int w{};
        int h{};
    };
    /**
     * @brief Class for managing drawing operations.
     * @note This class is used for drawing shapes, images, and text.
     */
    class draw_manager {
        int w{}, h{};
        protocol proto{protocol::unknown};
#if LIMHAMN_PRIMITIVE_X11
        x11_window xwin{};
#endif
#if LIMHAMN_PRIMITIVE_CANVAS
        canvas_window canvas_win{};
#endif
        image_manager img_manager{};
        font_manager font{};
        cairo_surface_t* surface{};
        cairo_t* d{};

        static void cairo_set_source_hex(cairo_t* cr, const std::string& col, int alpha);
    public:
        explicit draw_manager() = default;
#if LIMHAMN_PRIMITIVE_CANVAS
        /**
         * @brief Initializes the draw manager with a canvas.
         * @param data Pointer to the canvas data.
         * @param w Width of the canvas.
         * @param h Height of the canvas.
         */
        void initialize(void* data, int w, int h);
#endif
#if LIMHAMN_PRIMITIVE_X11
        /**
         * @brief Initializes the draw manager with X11.
         * @param dpy Pointer to the X11 display.
         * @param screen Screen number.
         * @param root Root window.
         * @param w Width of the drawable.
         * @param h Height of the drawable.
         * @param visual Pointer to the visual.
         * @param depth Depth of the drawable.
         * @param cmap Colormap.
         */
        void initialize_x11(Display* dpy, int screen, Window root, unsigned int w, unsigned int h, Visual *visual, unsigned int depth, Colormap cmap);
        /**
         * @brief Initializes the draw manager with X11.
         * @param dpy Pointer to the X11 display.
         * @param screen Screen number.
         * @param root Root window.
         * @param w Width of the drawable.
         * @param h Height of the drawable.
         * @param visual Pointer to the visual.
         * @param depth Depth of the drawable.
         * @param cmap Colormap.
         */
        draw_manager(Display* dpy, int screen, Window root, unsigned int w, unsigned int h, Visual *visual, unsigned int depth, Colormap cmap);
#endif
#if LIMHAMN_PRIMITIVE_CANVAS
        /**
         * @brief Initializes the draw manager with a canvas.
         * @param data Pointer to the canvas data.
         * @param w Width of the canvas.
         * @param h Height of the canvas.
         */
        draw_manager(void* data, int w, int h);
#endif
        /**
         * @brief Resizes the drawing area.
         * @param size The new size of the drawing area.
         */
        void resize(const draw_size& size);
        /**
         * @brief Draws an image on the canvas.
         * @param data Pointer to the image data.
         * @param coords The coordinates where the image will be drawn.
         */
        void draw_image(void* data, const draw_position& coords);
        /**
         * @brief Draws an arrow shape on the canvas.
         * @param pos The position and size of the arrow.
         * @param direction The direction of the arrow (0 or 1).
         * @param slash The slash direction (0 or 1).
         * @param props The properties of the arrow shape.
         */
        void draw_arrow(const draw_position& pos, int direction, int slash, const draw_shape_properties& props) const;
        /**
         * @brief Draws a circle shape on the canvas.
         * @param pos The position and size of the circle.
         * @param direction The direction of the circle (0 or 1).
         * @param props The properties of the circle shape.
         */
        void draw_circle(const draw_position& pos, int direction, const draw_shape_properties& props) const;
        /**
         * @brief Draws a rectangle shape on the canvas.
         * @param pos The position and size of the rectangle.
         * @param props The properties of the rectangle shape.
         */
        void draw_rect(const draw_position& pos, const draw_properties& props) const;
#if LIMHAMN_PRIMITIVE_X11
        /*
         * @brief Map the drawable to a window.
         * @param win The window to map the drawable to.
         * @note This function only needs to be called for X11 protocol. Do it when the window has been created and preferably when you're done drawing.
         */
        void map(Window win) const;
#endif
        /**
         * @brief Maps the drawable to the screen.
         * @note Does not need to be called, but exists anyway.
         */
        void map() const;
        /**
         * @brief Saves the current screen to a file.
         * @param file The filename to save the screen to.
         */
        void save_screen(const std::string& file) const;
        /**
         * @brief Initializes the font manager with a specified font.
         * @param font The name of the font to be used. (e.g "Sans 12")
         * @note Call before using methods such as draw_text.
         */
        void initialize_font(const std::string& font);
        /**
         * @brief Draws text on the canvas.
         * @param pos The position and size of the text.
         * @param padding The padding around the text.
         * @param input_text The text to be drawn.
         * @param markup If true, markup is applied to the text.
         * @param props The properties of the text.
         * @return The width of the drawn text.
         */
        int draw_text(const draw_position& pos, int padding, const std::string& input_text, bool markup, const draw_properties& props);
        /**
         * @brief Gets the width of the text.
         * @param str The text string to measure.
         * @param markup If true, markup is applied to the text.
         * @note This function is pretty accurate, because it calls draw_text but does not draw anything. Therefore, it's not very fast and should not be used in a loop.
         * @note Initialize the font manager before using this function.
         */
        unsigned int get_text_width(const std::string& str, bool markup = false);
        /**
         * @brief Gets the width of the text, clamped to a maximum value.
         * @param str The text string to measure.
         * @param n The maximum width of the text.
         * @param markup If true, markup is applied to the text.
         * @note This function is pretty accurate, because it calls draw_text but does not draw anything. Therefore, it's not very fast and should not be used in a loop.
         * @note Initialize the font manager before using this function.
         */
        unsigned int get_text_width_clamp(const std::string& str, int n, bool markup = false);
        /**
         * @brief Gets the font manager as a reference.
         * @return A reference to the font manager.
         */
        [[nodiscard]] font_manager& get_font_manager();
        ~draw_manager();
    };
} // namespace limhamn::primitive

#ifdef LIMHAMN_PRIMITIVE_IMPL
inline void limhamn::primitive::font_manager::init_font(const std::string& font) {
    if (this->active) {
        throw std::runtime_error("FontManager already initialized");
    }

    active = true;

    PangoFontMap* fontmap{};
    PangoContext* context{};
    PangoFontDescription* desc{};
    PangoFontMetrics* metrics{};

    if (font.empty()) {
        throw std::invalid_argument("Font name cannot be empty");
    }

    fontmap = pango_cairo_font_map_new();
    context = pango_font_map_create_context(fontmap);
    desc = pango_font_description_from_string(font.c_str());

    this->layout = pango_layout_new(context);

    if (!this->layout) {
        throw std::runtime_error{"Failed to load fonts."};
    }

    pango_layout_set_font_description(this->layout, desc);

    metrics = pango_context_get_metrics(context, desc, pango_language_from_string ("en-us"));
    this->h = pango_font_metrics_get_height(metrics) / PANGO_SCALE;

    pango_font_metrics_unref(metrics);
    g_object_unref(context);
}
[[nodiscard]] inline std::pair<int,int> limhamn::primitive::font_manager::estimate_length(const std::string& text, const int length, bool markup) const {
    if (!this->layout) {
        throw std::runtime_error("FontManager not initialized");
    }
    if (text.empty()) {
        return {0, 0};
    }

    if (text.find('<') != std::string::npos && text.find('>') != std::string::npos) {
        markup = true;
    }

    PangoRectangle r;

    if (markup) {
        pango_layout_set_markup(this->layout, text.c_str(), static_cast<int>(length == -1 ? text.length() : length));
    } else {
        pango_layout_set_text(this->layout, text.c_str(), static_cast<int>(length == -1 ? text.length() : length));
    }

    pango_layout_get_extents(this->layout, nullptr, &r);

    if (markup) {
        pango_layout_set_attributes(this->layout, nullptr);
    }

    return {
        (r.width / PANGO_SCALE),
        this->h
    };
}
[[nodiscard]] inline PangoLayout& limhamn::primitive::font_manager::get_layout() {
    if (!this->layout) {
        throw std::runtime_error("FontManager not initialized");
    }
    return *this->layout;
}
inline limhamn::primitive::font_manager::font_manager(const std::string& font) {
    init_font(font);
}
[[nodiscard]] inline bool limhamn::primitive::font_manager::is_active() const {
    return this->active;
}
[[nodiscard]] inline unsigned int limhamn::primitive::font_manager::get_height() const {
    if (!this->layout) {
        throw std::runtime_error("FontManager not initialized");
    }
    return this->h;
}
inline limhamn::primitive::font_manager::~font_manager() {
    if (this->layout) {
        g_object_unref(this->layout);
    }
}
inline void limhamn::primitive::draw_manager::cairo_set_source_hex(cairo_t* cr, const std::string& col, int alpha) {
    if (col.empty() || col[0] != '#' || col.length() != 7) {
        throw std::invalid_argument("Invalid color format. Expected format: #RRGGBB");
    }

    unsigned int hex;
    std::istringstream iss(col.substr(1));
    iss >> std::hex >> hex;

    if (iss.fail()) {
        throw std::invalid_argument("Failed to parse color hex value");
    }

    double r = ((hex >> 16) & 0xFF) / 255.0;
    double g = ((hex >> 8) & 0xFF) / 255.0;
    double b = (hex & 0xFF) / 255.0;

    cairo_set_source_rgba(cr, r, g, b, alpha / 255.0);
}

#if LIMHAMN_PRIMITIVE_CANVAS
inline void limhamn::primitive::draw_manager::initialize(void* data, int w, int h) {
    if (!data || !w || !h) {
        throw std::invalid_argument("Invalid arguments to DrawManager constructor");
    }

    this->proto = protocol::canvas;
    this->canvas_win.data = data;
    this->w = w;
    this->h = h;

    this->surface = cairo_image_surface_create_for_data(static_cast<unsigned char*>(this->canvas_win.data), CAIRO_FORMAT_ARGB32, w, h, w * 4);
    this->d = cairo_create(this->surface);

    if (!this->d) {
        throw std::runtime_error("Failed to create cairo context");
    }
}
#endif
#if LIMHAMN_PRIMITIVE_X11
inline void limhamn::primitive::draw_manager::initialize_x11(Display* dpy, int screen, Window root, unsigned int w, unsigned int h, Visual *visual, unsigned int depth, Colormap cmap) {
    if (!dpy || !root || !visual || !depth) {
        throw std::invalid_argument("Invalid arguments to DrawManager constructor");
    }

    this->proto = protocol::x11;
    this->xwin.dpy = dpy;
    this->xwin.screen = screen;
    this->xwin.root = root;
    this->w = static_cast<int>(w);
    this->h = static_cast<int>(h);
    this->xwin.visual = visual;
    this->xwin.depth = depth;
    this->xwin.cmap = cmap;

    this->xwin.drawable = XCreatePixmap(dpy, root, w, h, depth);

    if (!this->xwin.drawable) {
        throw std::runtime_error("Failed to create pixmap");
    }

    this->xwin.gc = XCreateGC(dpy, this->xwin.drawable, 0, nullptr);

    if (!this->xwin.gc) {
        XFreePixmap(dpy, this->xwin.drawable);
        throw std::runtime_error("Failed to create graphics context");
    }
    XSetLineAttributes(dpy, this->xwin.gc, 1, LineSolid, CapButt, JoinMiter);
}
inline limhamn::primitive::draw_manager::draw_manager(Display* dpy, int screen, Window root, unsigned int w, unsigned int h, Visual *visual, unsigned int depth, Colormap cmap) {
    initialize_x11(dpy, screen, root, w, h, visual, depth, cmap);
}
#endif
#if LIMHAMN_PRIMITIVE_CANVAS
inline limhamn::primitive::draw_manager::draw_manager(void* data, int w, int h) {
    initialize(data, w, h);
}
#endif
inline void limhamn::primitive::draw_manager::resize(const draw_size& size) {
    this->w = size.w;
    this->h = size.h;
#if LIMHAMN_PRIMITIVE_X11
    if (this->proto == protocol::x11) {
        if (!this->xwin.drawable) {
            throw std::runtime_error("Drawable not initialized");
        }

        XFreePixmap(this->xwin.dpy, this->xwin.drawable);

        this->xwin.drawable = XCreatePixmap(this->xwin.dpy, this->xwin.root, size.w, size.h, this->xwin.depth);
        if (!this->xwin.drawable) {
            throw std::runtime_error("Failed to create new pixmap");
        }
    }
#endif
}
inline void limhamn::primitive::draw_manager::draw_image(void* data, const draw_position& coords) {
    if (data == nullptr) {
        throw std::invalid_argument("Image data cannot be null");
    }
    if (protocol::unknown == this->proto) {
        throw std::invalid_argument("Invalid protocol");
    }

    this->img_manager = {};
    this->img_manager.data = static_cast<uint8_t*>(data);

    for (int i = 0; i < coords.w * coords.h; i++) {
        uint8_t* px = &this->img_manager.data[i * 4];
        const uint8_t r = px[0];
        const uint8_t g = px[1];
        const uint8_t b = px[2];
        const uint8_t a = px[3];

        px[0] = (r * a) / 255;
        px[1] = (g * a) / 255;
        px[2] = (b * a) / 255;
        px[3] = a;
    }

    this->img_manager.img_surface =
        cairo_image_surface_create_for_data(this->img_manager.data,
            CAIRO_FORMAT_ARGB32,
            coords.w,
            coords.h,
            coords.w * 4
        );

    if (!this->img_manager.img_surface) {
        throw std::runtime_error("Failed to create image surface");
    }

    cairo_set_operator(this->d, CAIRO_OPERATOR_OVER);
    cairo_set_source_hex(this->d, "#ffffff", 255);

    cairo_set_source_surface(this->d, this->img_manager.img_surface, coords.x, coords.y);
    cairo_mask_surface(this->d, this->img_manager.img_surface, coords.x, coords.y);

    cairo_set_source_surface(this->d, this->surface, this->w, this->h);
}
inline void limhamn::primitive::draw_manager::draw_arrow(const draw_position& pos, int direction, int slash, const draw_shape_properties& props) const {
    int x = pos.x;
    int y = pos.y;
    int w = pos.w;
    int h = pos.h;

    x = direction ? x : x + w;
    w = direction ? w : - w;

    cairo_surface_t* sf = nullptr;

    if (this->proto == protocol::canvas) {
#if LIMHAMN_PRIMITIVE_CANVAS
        sf = cairo_image_surface_create_for_data(static_cast<unsigned char*>(this->canvas_win.data), CAIRO_FORMAT_ARGB32, this->w, this->h, this->w * 4);
#endif
    } else {
#if LIMHAMN_PRIMITIVE_X11
        sf = cairo_xlib_surface_create(this->xwin.dpy, this->xwin.drawable, this->xwin.visual, this->w, this->h);
#endif
    }

    if (!sf) {
        throw std::runtime_error("Failed to create surface");
    }

    double hh = slash ? (direction ? 0 : h) : h / 2;

    cairo_t* cr = cairo_create(sf);
    cairo_set_source_hex(cr, props.prev, props.prev_alpha);
    cairo_set_operator(cr, CAIRO_OPERATOR_SOURCE);

    cairo_rectangle(cr, x, y, w, h);
    cairo_fill(cr);

    cairo_move_to(cr, x, y);
    cairo_line_to(cr, x + w, y + hh);
    cairo_line_to(cr, x, y + h);
    cairo_close_path(cr);

    cairo_set_source_hex(cr, props.next, props.next_alpha);
    cairo_fill(cr);

    cairo_destroy(cr);
    cairo_surface_destroy(sf);
}
inline void limhamn::primitive::draw_manager::draw_circle(const draw_position& pos, int direction, const draw_shape_properties& props) const {
    cairo_surface_t *sf = nullptr;

    if (this->proto == protocol::canvas) {
#if LIMHAMN_PRIMITIVE_CANVAS
        sf = cairo_image_surface_create_for_data(static_cast<unsigned char*>(this->canvas_win.data), CAIRO_FORMAT_ARGB32, this->w, this->h, this->w * 4);
#endif
    } else {
#if LIMHAMN_PRIMITIVE_X11
        sf = cairo_xlib_surface_create(this->xwin.dpy, this->xwin.drawable, this->xwin.visual, this->w, this->h);
#endif
    }

    if (!sf) {
        throw std::runtime_error("Failed to create surface");
    }

    cairo_t *cr = cairo_create(sf);

    cairo_set_source_hex(cr, props.prev, props.prev_alpha);
    cairo_set_operator(cr, CAIRO_OPERATOR_SOURCE);

    cairo_rectangle(cr, pos.x, pos.y, pos.w, pos.h);
    cairo_fill(cr);

    double cx = direction ? pos.x + pos.w - pos.h / 2.0 : pos.x + pos.h / 2.0;
    double cy = pos.y + pos.h / 2;
    double rad = pos.h / 2.0;

    double start = direction ? -M_PI_2 : M_PI_2;
    double end = direction ? M_PI_2 : 3 * M_PI_2;

    cairo_arc(cr, cx, cy, rad, start, end);
    cairo_close_path(cr);

    cairo_set_source_hex(cr, props.next, props.next_alpha);
    cairo_fill(cr);

    cairo_destroy(cr);
    cairo_surface_destroy(sf);
}
inline void limhamn::primitive::draw_manager::draw_rect(const draw_position& pos, const draw_properties& props) const {
    cairo_surface_t *sf;

    if (this->proto == protocol::canvas) {
#if LIMHAMN_PRIMITIVE_CANVAS
        sf = cairo_image_surface_create_for_data(static_cast<unsigned char*>(this->canvas_win.data), CAIRO_FORMAT_ARGB32, this->w, this->h, this->w * 4);
#endif
    } else {
#if LIMHAMN_PRIMITIVE_X11
        sf = cairo_xlib_surface_create(this->xwin.dpy, this->xwin.drawable, this->xwin.visual, this->w, this->h);
#endif
    }

    cairo_t *cr = cairo_create(sf);

    if (!cr) {
        throw std::runtime_error("Failed to create cairo context");
    }

    cairo_set_source_hex(cr, props.invert ? props.background.c_str() : props.foreground.c_str(), props.invert ? props.background_alpha : props.foreground_alpha);
    cairo_set_operator(cr, CAIRO_OPERATOR_SOURCE);

    if (props.filled) {
        cairo_rectangle(cr, pos.x, pos.y, pos.w, pos.h);
        cairo_fill(cr);
    } else {
        cairo_rectangle(cr, pos.x, pos.y, pos.w - 1, pos.h - 1);
        cairo_stroke(cr);
    }

    cairo_destroy(cr);
    cairo_surface_destroy(sf);
}
#if LIMHAMN_PRIMITIVE_X11
inline void limhamn::primitive::draw_manager::map(Window win) const {
    if (this->proto == protocol::x11) {
        if (!this->xwin.drawable) {
            throw std::runtime_error("Drawable not initialized");
        }
        XCopyArea(this->xwin.dpy, this->xwin.drawable, win, this->xwin.gc, 0, 0, this->w, this->h, 0, 0);
        XFlush(this->xwin.dpy);
    } else {
        throw std::runtime_error("Mapping not supported for this protocol");
    }
}
#endif
inline void limhamn::primitive::draw_manager::map() const {
    if (this->proto == protocol::x11) {
        throw std::runtime_error("X11 must be called with a window");
    }
}
inline void limhamn::primitive::draw_manager::save_screen(const std::string& file) const {
    if (file.empty()) {
        throw std::invalid_argument("Invalid filename");
    }

    if (!this->surface) {
        throw std::runtime_error("No surface to save");
    }

    if (cairo_surface_write_to_png(this->surface, file.c_str()) != CAIRO_STATUS_SUCCESS) {
        throw std::runtime_error("Failed to save screen");
    }
}
inline void limhamn::primitive::draw_manager::initialize_font(const std::string& font) {
    if (font.empty()) {
        throw std::invalid_argument("Font name cannot be empty");
    }

    this->font.init_font(font);
}
inline int limhamn::primitive::draw_manager::draw_text(const draw_position& pos, int padding, const std::string& input_text, bool markup, const draw_properties& props) {
    int x = pos.x;
    int y = pos.y;
    unsigned int w = pos.w;
    unsigned int h = pos.h;

    const int render = x || y || w || h;

    if (!this->font.is_active()) {
        throw std::runtime_error("FontManager not initialized");
    }

    if (!render) {
        w = ~w;
    } else {
        x += padding;
        w -= padding;

        if (proto == protocol::canvas) {
#if LIMHAMN_PRIMITIVE_CANVAS
            this->surface = cairo_image_surface_create_for_data(static_cast<unsigned char*>(this->canvas_win.data), CAIRO_FORMAT_ARGB32, this->w, this->h, this->w * 4);
#endif
        } else {
#if LIMHAMN_PRIMITIVE_X11
            this->surface = cairo_xlib_surface_create(this->xwin.dpy, this->xwin.drawable, this->xwin.visual, this->w, this->h);
#endif
        }

        this->d = cairo_create(this->surface);

        cairo_set_source_hex(this->d, props.invert ? props.foreground.c_str() : props.background.c_str(), props.invert ? props.foreground_alpha : props.background_alpha);
        cairo_set_operator(this->d, CAIRO_OPERATOR_SOURCE);
        cairo_rectangle(this->d, x - padding, y, w + padding, h);
        cairo_fill(this->d);
    }

    // HACK: guess that there is no markup if there is no presence of "</"
    // not sure if it's really even necessary
    if (input_text.find("</") == std::string::npos) {
        markup = false;
    }

    if (input_text.empty()) {
        return x + (render ? w : 0);
    }

    int length = static_cast<int>(input_text.length());
    int estimated_width = font.estimate_length(input_text, input_text.length(), markup).first;

    for (length = std::min(length, static_cast<int>(input_text.length())); length && estimated_width > w; font.estimate_length(input_text, length--, markup).first) {
        estimated_width = font.estimate_length(input_text, length, markup).first;
    }

    if (!length) {
        return x + (render ? w : 0);
    }

    std::string text = input_text.substr(0, length);
    if (length < static_cast<int>(input_text.length())) {
        for (std::size_t i = length; i && i > length - 3; text[--i] = '.')
            ; // NOP
    }

    estimated_width = font.estimate_length(text, -1, markup).first;

    // HACK: guess that there is no markup if there is no presence of "</"
    // not sure if it's really even necessary
    if (text.find("</") == std::string::npos) {
        markup = false;
    }

    if (!render) {
        return x + estimated_width;
    }

    if (markup) {
        pango_layout_set_markup(this->font.layout, text.c_str(), length);
    } else {
        pango_layout_set_text(this->font.layout, text.c_str(), length);
    }

    pango_layout_set_single_paragraph_mode(this->font.layout, true);

    cairo_set_source_hex(this->d, props.foreground, props.foreground_alpha);
    cairo_move_to(this->d, x, y + (h - this->font.get_height()) / 2);

    pango_cairo_update_layout(this->d, this->font.layout);
    pango_cairo_show_layout(this->d, this->font.layout);

    cairo_set_operator(this->d, CAIRO_OPERATOR_SOURCE);

    if (markup) {
        pango_layout_set_attributes(this->font.layout, nullptr);
    }

    return (x + estimated_width) + (w - estimated_width);
}
inline unsigned int limhamn::primitive::draw_manager::get_text_width(const std::string& str, bool markup) {
    try {
        return draw_text({0, 0, 0, 0}, 0, str, markup, {"#000000", "#000000", 0, 0});
    } catch (const std::exception&) {
        return 0;
    }
}
inline unsigned int limhamn::primitive::draw_manager::get_text_width_clamp(const std::string& str, int n, bool markup) {
    try {
        return std::min(n, draw_text({0, 0, 0, 0}, 0, str, markup, {"#000000", "#000000", 0, 0}));
    } catch (const std::exception&) {
        return n;
    }
}
inline limhamn::primitive::font_manager& limhamn::primitive::draw_manager::get_font_manager() {
    return this->font;
}
inline limhamn::primitive::draw_manager::~draw_manager() {
#if LIMHAMN_PRIMITIVE_X11
    if (this->proto == protocol::unknown && this->xwin.drawable) {
        XFreePixmap(this->xwin.dpy, this->xwin.drawable);
    }
    if (this->proto == protocol::x11 && this->xwin.gc) {
        XFreeGC(this->xwin.dpy, this->xwin.gc);
    }
#endif
}
#endif // LIMHAMN_PRIMITIVE_IMPL
