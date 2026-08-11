// ─── sliceofpy C++ Tests ────────────────────────────────────────────────────
// Mirrors test_math_utils.py and test_slicer.py

#include <iostream>
#include <cmath>
#include <cassert>
#include <string>
#include "math_utils.h"
#include "slicer.h"

using namespace std;

static bool approx_eq(float a, float b, float eps = 1e-5f) { return fabs(a - b) < eps; }
static bool vertex_eq(const Vertex& a, const Vertex& b) {
    return approx_eq(a.x, b.x) && approx_eq(a.y, b.y) && approx_eq(a.z, b.z);
}

static int tests_passed = 0;
static int tests_failed = 0;

#define TEST(name)  do { cout << "  TEST: " << #name << " ... "; } while(0)
#define PASS()      do { cout << "PASSED" << endl; ++tests_passed; } while(0)
#define FAIL(msg)   do { cout << "FAILED: " << msg << endl; ++tests_failed; } while(0)

void test_get_intersection() {
    cout << "\n=== test_get_intersection ===" << endl;
    Vertex c1(1, 1, 1);

    { TEST(intersection_x); Vertex c2(3,1,1);
      if (vertex_eq(get_intersection(c1, c2, Axis::X, 2.0f), Vertex(2,1,1))) PASS(); else FAIL("Expected (2,1,1)"); }
    { TEST(intersection_y); Vertex c2(1,3,1);
      if (vertex_eq(get_intersection(c1, c2, Axis::Y, 2.0f), Vertex(1,2,1))) PASS(); else FAIL("Expected (1,2,1)"); }
    { TEST(intersection_z); Vertex c2(1,1,3);
      if (vertex_eq(get_intersection(c1, c2, Axis::Z, 2.0f), Vertex(1,1,2))) PASS(); else FAIL("Expected (1,1,2)"); }
}

void test_distance_between() {
    cout << "\n=== test_distance_between ===" << endl;
    Vertex c1(1, 1, 1);

    { TEST(distance_x); Vertex c2(3,1,1);
      if (approx_eq(distance_between(c1,c2),2.0f) && approx_eq(distance_between(c2,c1),2.0f)) PASS(); else FAIL("Expected 2.0"); }
    { TEST(distance_y); Vertex c2(1,3,1);
      if (approx_eq(distance_between(c1,c2),2.0f) && approx_eq(distance_between(c2,c1),2.0f)) PASS(); else FAIL("Expected 2.0"); }
    { TEST(distance_z); Vertex c2(1,1,3);
      if (approx_eq(distance_between(c1,c2),2.0f) && approx_eq(distance_between(c2,c1),2.0f)) PASS(); else FAIL("Expected 2.0"); }
}

void test_generate_gcode(const string& test_data_dir, const string& template_dir) {
    cout << "\n=== test_generate_gcode ===" << endl;

    auto run_test = [&](const string& name, const string& obj_file) {
        cout << "  TEST: " << name << " ... ";
        try {
            generate_gcode(test_data_dir + "/" + obj_file, "test_" + obj_file + ".gcode",
                0.2f, 1.0f, 3600.0f, -1.0f, 1.75f, 0.4f, 1.0f, "cross", 5.0f, 3,
                "PLA", "PLA", "mm", 0.1f, template_dir);
            PASS();
        } catch (const exception& e) { FAIL(e.what()); }
    };

    run_test("block",    "block.obj");
    run_test("pyramid",  "pyramid.obj");
    run_test("ring",     "ring.obj");
    run_test("2block",   "2block.obj");
    run_test("icecream", "icecream.obj");
    run_test("torus",    "torus.obj");
}

int main(int argc, char* argv[]) {
    string test_data_dir = "../../tests";
    string template_dir  = "../../sliceofpy/templates/";
    if (argc >= 3) { test_data_dir = argv[1]; template_dir = argv[2]; }

    cout << "sliceofpy C++ Test Suite" << endl;
    cout << "========================" << endl;

    test_get_intersection();
    test_distance_between();
    test_generate_gcode(test_data_dir, template_dir);

    cout << "\n========================" << endl;
    cout << "Results: " << tests_passed << " passed, " << tests_failed << " failed" << endl;
    return tests_failed > 0 ? 1 : 0;
}
