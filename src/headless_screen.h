#ifndef HEADLESS_SCREEN_H
#define HEADLESS_SCREEN_H

#include <vector>
#include <string>
#include <memory>
#include <fstream>
#include <sstream>
#include <stack>

/* 
 * HeadlessScreen: A recording screen implementation that captures all graphics operations
 * instead of rendering to actual display.
 */

struct GraphicsCall {
    std::string function_name;
    std::string type;  // "primitive", "sprite", "text", "bitmap", etc.
    void* address;     // Memory address related to the operation
    int x, y, w, h;    // Dimensions
    size_t size;       // Size of data in bytes
    std::vector<std::string> args;  // Additional arguments
    
    GraphicsCall(const std::string& func, const std::string& t, void* addr, 
                 int x1 = 0, int y1 = 0, int w1 = 0, int h1 = 0, size_t s = 0)
        : function_name(func), type(t), address(addr), x(x1), y(y1), w(w1), h(h1), size(s) {}
};

struct RenderScope {
    std::string name;
    std::vector<GraphicsCall> calls;
    std::vector<std::unique_ptr<RenderScope>> sub_scopes;
    
    RenderScope(const std::string& n) : name(n) {}
};

struct GameLoopIteration {
    int iteration_number;
    RenderScope render_root;
    std::vector<std::string> diagnostics;  // Any diagnostic info
    
    GameLoopIteration(int num) : iteration_number(num), render_root("render") {}
};

class HeadlessScreen {
private:
    static HeadlessScreen* instance;
    
    // Current state
    std::vector<RenderScope*> scope_stack;  // Raw pointer stack, managed by unique_ptr in parent containers
    std::vector<std::unique_ptr<GameLoopIteration>> iterations;
    int current_iteration_num;
    
    // Output configuration
    std::string output_path;
    bool output_enabled;
    
    // Private constructor for singleton
    HeadlessScreen();
    
public:
    static HeadlessScreen* get_instance();
    
    // Initialize with output path
    void init(const std::string& path = "");
    
    // Start a new iteration
    void start_iteration(int iteration_num);
    
    // Start a new render scope (e.g., "draw_sprite", "draw_text", etc.)
    void push_render_scope(const std::string& name);
    
    // End current render scope
    void pop_render_scope();
    
    // Record a graphics call
    void record_call(const std::string& function_name, const std::string& type, 
                     void* address, int x = 0, int y = 0, int w = 0, int h = 0, 
                     size_t size = 0, const std::vector<std::string>& args = {});
    
    // Add diagnostic information
    void add_diagnostic(const std::string& msg);
    
    // End the current iteration
    void end_iteration();
    
    // Clear the screen (in headless mode, this just resets internal state)
    void clear();
    
    // Flush all recorded data to output
    void flush_output();
    
    // Export to JSON string
    std::string to_json() const;
    
    // Getters for debugging
    const std::vector<std::unique_ptr<GameLoopIteration>>& get_iterations() const { return iterations; }
    RenderScope* get_current_scope();
    
private:
    // Helper methods for JSON output
    std::string escape_json_string(const std::string& str) const;
    std::string graphics_call_to_json(const GraphicsCall& call) const;
    std::string render_scope_to_json(const RenderScope& scope) const;
    std::string iteration_to_json(const GameLoopIteration& iteration) const;
    
    // Utility functions for size calculation (moved to public section)
    
public:
    // Utility functions for size calculation
    static size_t calculate_bitmap_size(int width, int height, int bpp = 32);
};

// Convenience macros for recording graphics calls
#define RECORD_CALL(func, type, addr, x, y, w, h, size, ...) \
    if (HeadlessScreen* hs = HeadlessScreen::get_instance()) { \
        std::vector<std::string> args = {__VA_ARGS__}; \
        hs->record_call(func, type, addr, x, y, w, h, size, args); \
    }

#define PUSH_RENDER_SCOPE(name) \
    if (HeadlessScreen* hs = HeadlessScreen::get_instance()) { \
        hs->push_render_scope(name); \
    }

#define POP_RENDER_SCOPE() \
    if (HeadlessScreen* hs = HeadlessScreen::get_instance()) { \
        hs->pop_render_scope(); \
    }

#endif // HEADLESS_SCREEN_H