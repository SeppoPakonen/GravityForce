#include "headless_screen.h"
#include <iostream>
#include <iomanip>
#include <algorithm>
#include <cctype>
#include <sstream>
#include <fstream>
#include <cstdio>

// Static instance for singleton
HeadlessScreen* HeadlessScreen::instance = nullptr;

HeadlessScreen::HeadlessScreen() : current_iteration_num(0), output_enabled(false) {
    // Initially we don't push anything to the stack - it will be populated during iteration
}

HeadlessScreen* HeadlessScreen::get_instance() {
    if (!instance) {
        instance = new HeadlessScreen();
    }
    return instance;
}

void HeadlessScreen::init(const std::string& path) {
    output_path = path;
    output_enabled = !path.empty();
    
    if (output_enabled) {
        std::cout << "HeadlessScreen initialized with output to: " << path << std::endl;
    }
}

void HeadlessScreen::start_iteration(int iteration_num) {
    current_iteration_num = iteration_num;
    
    // Create a new iteration
    std::unique_ptr<GameLoopIteration> iteration(new GameLoopIteration(iteration_num));
    iterations.push_back(std::move(iteration));
    
    // Reset scope stack to only have root render scope
    scope_stack.clear();
    
    // Add the root render scope
    GameLoopIteration* current_iter = iterations.back().get();
    RenderScope* root_scope = &(current_iter->render_root);
    scope_stack.push_back(root_scope);
}

void HeadlessScreen::push_render_scope(const std::string& name) {
    std::unique_ptr<RenderScope> new_scope(new RenderScope(name));
    
    // Add this scope to the current scope's sub-scopes
    if (!scope_stack.empty()) {
        RenderScope* current = scope_stack.back();
        // Keep a reference to the raw pointer before moving
        RenderScope* raw_ptr = new_scope.get();
        current->sub_scopes.push_back(std::move(new_scope));
        // Now make this new scope the current one by pushing its raw pointer to the stack
        scope_stack.push_back(raw_ptr);
    } else if (!iterations.empty()) {
        // If no scope exists, create directly in current iteration
        GameLoopIteration* current_iter = iterations.back().get();
        // Keep a reference to the raw pointer before moving
        RenderScope* raw_ptr = new_scope.get();
        current_iter->render_root.sub_scopes.push_back(std::move(new_scope));
        scope_stack.push_back(raw_ptr);
    }
}

void HeadlessScreen::pop_render_scope() {
    if (!scope_stack.empty() && scope_stack.size() > 1) { // Don't pop the root render scope
        scope_stack.pop_back();
    }
}

void HeadlessScreen::record_call(const std::string& function_name, 
                                 const std::string& type, 
                                 void* address, 
                                 int x, int y, int w, int h, 
                                 size_t size, 
                                 const std::vector<std::string>& args) {
    if (!scope_stack.empty()) {
        GraphicsCall call(function_name, type, address, x, y, w, h, size);
        call.args = args;
        scope_stack.back()->calls.push_back(call);
    }
}

void HeadlessScreen::add_diagnostic(const std::string& msg) {
    if (!iterations.empty()) {
        iterations.back()->diagnostics.push_back(msg);
    }
}

void HeadlessScreen::end_iteration() {
    // The iteration is already added when start_iteration is called
    // Just ensure the current scope stack is properly handled
    scope_stack.clear();
    
    // Re-add the root render scope for next iteration if we have iterations
    if (!iterations.empty()) {
        GameLoopIteration* current_iter = iterations.back().get();
        RenderScope* root_scope = &(current_iter->render_root);
        scope_stack.push_back(root_scope);
    }
}

void HeadlessScreen::clear() {
    // In headless mode, just reset internal state
    // This doesn't actually clear a bitmap since we're not rendering
}

RenderScope* HeadlessScreen::get_current_scope() {
    if (!scope_stack.empty()) {
        return scope_stack.back();
    }
    return nullptr;
}

size_t HeadlessScreen::calculate_bitmap_size(int width, int height, int bpp) {
    // Calculate size in bytes for a bitmap with given dimensions and bits per pixel
    size_t pixels = static_cast<size_t>(width) * static_cast<size_t>(height);
    size_t bytes_per_pixel = bpp / 8;
    return pixels * bytes_per_pixel;
}

std::string HeadlessScreen::to_json() const {
    std::ostringstream json;
    json << "{\n  \"headless_screen_data\": {\n    \"iterations\": [";
    
    for (size_t i = 0; i < iterations.size(); ++i) {
        if (i > 0) json << ",";
        json << "\n      " << iteration_to_json(*(iterations[i]));
    }
    
    if (!iterations.empty()) json << "\n    ";
    json << "]\n  }\n}";
    
    return json.str();
}

std::string HeadlessScreen::escape_json_string(const std::string& str) const {
    std::string result;
    for (char c : str) {
        switch (c) {
            case '"': result += "\\\""; break;
            case '\\': result += "\\\\"; break;
            case '\b': result += "\\b"; break;
            case '\f': result += "\\f"; break;
            case '\n': result += "\\n"; break;
            case '\r': result += "\\r"; break;
            case '\t': result += "\\t"; break;
            default:
                if (c >= 0 && c < 32) {
                    char buf[8];
                    snprintf(buf, sizeof(buf), "\\u%04x", c);
                    result += buf;
                } else {
                    result += c;
                }
                break;
        }
    }
    return result;
}

std::string HeadlessScreen::graphics_call_to_json(const GraphicsCall& call) const {
    std::ostringstream json;
    json << "{\n";
    json << "        \"function_name\": \"" << escape_json_string(call.function_name) << "\",\n";
    json << "        \"type\": \"" << escape_json_string(call.type) << "\",\n";
    json << "        \"address\": \"" << call.address << "\",\n";
    json << "        \"x\": " << call.x << ",\n";
    json << "        \"y\": " << call.y << ",\n";
    json << "        \"w\": " << call.w << ",\n";
    json << "        \"h\": " << call.h << ",\n";
    json << "        \"size\": " << call.size << ",\n";
    
    json << "        \"args\": [";
    for (size_t i = 0; i < call.args.size(); ++i) {
        if (i > 0) json << ", ";
        json << "\"" << escape_json_string(call.args[i]) << "\"";
    }
    json << "]\n";
    
    json << "      }";
    
    return json.str();
}

std::string HeadlessScreen::render_scope_to_json(const RenderScope& scope) const {
    std::ostringstream json;
    json << "{\n";
    json << "        \"name\": \"" << escape_json_string(scope.name) << "\",\n";
    json << "        \"calls\": [";
    
    for (size_t i = 0; i < scope.calls.size(); ++i) {
        if (i > 0) json << ",";
        json << graphics_call_to_json(scope.calls[i]);
    }
    
    if (!scope.calls.empty()) json << "\n        ";
    json << "],\n";
    json << "        \"sub_scopes\": [";
    
    for (size_t i = 0; i < scope.sub_scopes.size(); ++i) {
        if (i > 0) json << ",";
        json << render_scope_to_json(*(scope.sub_scopes[i]));
    }
    
    if (!scope.sub_scopes.empty()) json << "\n        ";
    json << "]\n";
    json << "      }";
    
    return json.str();
}

std::string HeadlessScreen::iteration_to_json(const GameLoopIteration& iteration) const {
    std::ostringstream json;
    json << "{\n";
    json << "      \"iteration_number\": " << iteration.iteration_number << ",\n";
    json << "      \"render\": " << render_scope_to_json(iteration.render_root) << ",\n";
    json << "      \"diagnostics\": [";
    for (size_t i = 0; i < iteration.diagnostics.size(); ++i) {
        if (i > 0) json << ", ";
        json << "\"" << escape_json_string(iteration.diagnostics[i]) << "\"";
    }
    json << "]\n";
    json << "    }";
    
    return json.str();
}

void HeadlessScreen::flush_output() {
    if (!output_enabled || output_path.empty()) {
        return;
    }
    
    std::ofstream output_file(output_path);
    if (output_file.is_open()) {
        output_file << to_json();
        output_file.close();
        std::cout << "HeadlessScreen data written to: " << output_path << std::endl;
    } else {
        std::cerr << "Error: Could not open output file: " << output_path << std::endl;
    }
}