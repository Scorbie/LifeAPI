#include "LifeAPI.h"
#include <iostream>

void testWithMsg(bool (*test_fn)(), const char* test_msg)
{
    std::cout
        << test_msg << ": " << std::flush
        << (test_fn() ? "Success" : "Fail!")
        << std::endl;
}

bool testInit01()
{
    LifeState a;
    LifeState b;
    b.clear();
    return a == b;
}

bool testInit02()
{
    LifeState a;
    LifeState b(a);
    return a == b;
}

bool testInitRLE01()
{
    LifeState a("o$o$bo!");
    return a.getCell(0, 0) && a.getCell(0, 1) && a.getCell(1, 2);
}

bool testInitRLE02()
{
    LifeState a("bo$2bo$3o!"); // Canonical Glider
    LifeState b(a);
    a.run(4);
    b = b.transform(1, 1);
    return a == b;
}

bool testInitRLE03()
{
    LifeState a("boo$oob$bbo!");
    LifeState b("b2o$2ob$2bo!");
    return a == b;
}

bool testInitRLE04()
{
    LifeState a("boo$3o$bob");
    LifeState b("b2o$3o$bob!");
    return a == b;
}

bool testOpAnd01()
{
    LifeState a("bo$2bo$3o!"); // Canonical Glider starting at (0, 0)
    LifeState b(a);
    LifeState c("$2bo$3bo$b3o!"); // Glider at (1, 1)
    LifeState ab(a);
    LifeState ac("$2bo!");
    b &= a;
    c &= a;
    return (b == ab) && (c == ac);
}

bool testOpAnd02()
{
    LifeState a("bo$2bo$3o!"); // Canonical Glider starting at (0, 0)
    LifeState b(a);
    LifeState ab(a);
    LifeState c("$2bo$3bo$b3o!"); // Glider at (1, 1)
    LifeState ac("$2bo!");
    return ((a & b) == ab) && ((a & c) == ac);
}

bool testOpOr01()
{
    LifeState a("bo$2bo$3o!"); // Canonical Glider starting at (0, 0)
    LifeState b(a);
    LifeState c("$2bo$3bo$b3o!"); // Glider at (1, 1)
    LifeState ab(a);
    LifeState ac("bo$2bo$4o$b3o!");
    b |= a;
    c |= a;
    return (b == ab) && (c == ac);
}

bool testOpOr02()
{
    LifeState a("bo$2bo$3o!"); // Canonical Glider starting at (0, 0)
    LifeState b(a);
    LifeState c("$2bo$3bo$b3o!"); // Glider at (1, 1)
    LifeState ab(a);
    LifeState ac("bo$2bo$4o$b3o!");
    return ((a | b) == ab) && ((a | c) == ac);
}

bool testOpXor01()
{
    LifeState a("bo$2bo$3o!"); // Canonical glider starting at (0, 0)
    LifeState b(a);
    LifeState c("bo$2bo$3o!", 1, 1); // Glider at (1, 1)
    LifeState xab; // Empty LifeState
    LifeState xac("bo2$4o$b3o!");
    b ^= a;
    c ^= a;
    return (b == xab) && (c == xac);
}

bool testOpXor02()
{
    LifeState a("bo$2bo$3o!"); // Canonical glider starting at (0, 0)
    LifeState b(a);
    LifeState c("bo$2bo$3o!", 1, 1); // Glider at (1, 1)
    LifeState xab; // Empty LifeState
    LifeState xac("bo2$4o$b3o!");
    return ((a ^ b) == xab) && ((a ^ c) == xac);
}

bool testOpPlusMinus()
{
    LifeState a("bo$2bo$3o!");
    LifeState b = a.transform(1, 1);
    LifeState c("bo2$3o!");
    LifeState d(a);
    d.setCell(2, 1, 0);
    return ((a + b) == (a | b) && (a - b) == c && (a - b) == d);
}

bool testTransform01()
{
    LifeState c("b2o$o2bo$o2bo$b2o!");
    LifeState ct1 = c.transform(0, 0, -1, 0, 0, -1);
    LifeState ct2 = c.transform(-3, -3);
    c.move(-3, -3);
    return (c == ct1) && (c == ct2);
}


bool testTransform02()
{
    // Original patterns
    LifeState a("bo$2bo$3o!"); // Canonical glider
    LifeState b("3o$2bo$bo!"); // NE-headed glider
    // Transformed variants
    LifeState ta = a.transform(2, 2, -1, 0, 0, -1);
    LifeState tb = b.transform(2, 0, -1, 0, 0, 1);
    return (ta == tb);
}

// Copied from LifeState::removeGliders...
static const LifeState glider("bo$2bo$3o!", -2, -2);

bool testLifeLocator01()
{
    CellList wanted = glider.toLifeLocator().withBoundary().wanted;
    return (wanted.toLifeState() == glider);
}

bool testLifeLocator02()
{
    CellList unwanted = glider.toLifeLocator().withBoundary().unwanted;
    LifeState glider_unwanted("b3o$bob2o$3obo$o3bo$5o!", -3, -3);
    return (unwanted.toLifeState() == glider_unwanted);
}

bool testRemoveGliders01()
{
    bool status = true;
    LifeState a = glider.transform(0, 0, 0, -1, 1, 0); // SW
    for (int i=0; i<150; i++)
    {
        a.run();
        int pop = a.getPop();
        if (pop != 5 and pop != 0)
        {
            std::cerr << a.toDebugString() << std::endl;
            status = false;
        }
    }
    if (a.getPop() != 0)
    {
        std::cerr << a.toDebugString() << std::endl;
        status = false;
    }
    std::vector<GliderData> a_gliders = a.getGliders();
    if (a_gliders.size() == 1)
    {
        GliderData a_glider = a_gliders[0];
        bool glider_data_test = (
            a_glider.gen == 128
            and a_glider.x == -32 and a_glider.y == -32
            and a_glider.dx == false and a_glider.dy == true
        );
        status = status and glider_data_test;
    }
    else
    {
        status = false;
    }
    return status;
}

bool testPatternMatching01()
{
    // The objects
    LifeState gliders;
    LifeState locations;
    int points[][2] = {
        {2, 15},
        {3, -7},
        {-5, 6},
    };
    size_t num_points = (sizeof points) / (sizeof points[0]);
    // Setup code
    for(size_t i=0; i<num_points; ++i)
    {
        int x = points[i][0];
        int y = points[i][1];
        gliders |= glider.transform(x, y);
        locations.setCell(x, y, 1);
    }
    // Calculate the location of the gliders.
    LifeState result = gliders.locate(glider.toLifeLocator());
    return (result == locations);
}

bool testPatternMatching02()
{
    // The objects
    LifeState gliders;
    int points[][2] = {
        {2, 15},
        {3, -7},
        {-5, 6},
    };
    size_t num_points = (sizeof points) / (sizeof points[0]);
    // Setup code
    for(size_t i=0; i<num_points; ++i)
    {
        int x = points[i][0];
        int y = points[i][1];
        gliders |= glider.transform(x, y);
    }
    // Calculate the location of the gliders.
    gliders.remove(glider.toLifeLocator());
    return (gliders == LifeState());
}

// Tests from UnitTest.c

// Test 01. Find 2B+G collisions that return one or more gliders.
bool testSimkin01()
{
    int num_results = 0;
    LifeState block("2o$2o!");
    CellList rect_b = LifeState::makeRect(-10, -10, 20, 10).toCellList();
    CellList rect_g = LifeState::makeRect(-10, -10, 35, 1).toCellList();
    for (CellList::iterator it_b=rect_b.begin(); it_b != rect_b.end(); ++it_b)
    {
        for(CellList::iterator it_g=rect_g.begin(); it_g != rect_g.end(); ++it_g)
        {
            LifeState state(block);
            state |= block.transform(it_b->x, it_b->y);
            state |= glider.transform(it_g->x, it_g->y);
            LifeState backup(state);
            // Remove unstable patterns like 2o$4o$2b2o!
            bool valid = true;
            for (int i=0; i<4; ++i)
            {
                state.run();
                if (state.getPop() != 5 + 4 + 4)
                { 
                    valid = false;
                    break;
                }
            }
            if (!valid)
            {
                continue;
            }
            state.run(200);
            if (state.getPop() == 0 and state.getGliders().size() > 0)
            {
                num_results++;
                // std::cerr << backup.toRLE() << std::endl;
            }
        }
    }
    // The results:
    // 20$25bo$26bo$24b3o4$36b2o$36b2o5$32b2o$32b2o
    // 20$28bo$29bo$27b3o5$36b2o$36b2o4$32b2o$32b2o
    // 20$24bo$25bo$23b3o6$37b2o$37b2o3$32b2o$32b2o
    // 20$28bo$29bo$27b3o6$38b2o$38b2o3$32b2o$32b2o
    return (num_results == 4);
}

// Test 02. Add a glider to the pattern to make a dart.
bool testSimkin02()
{
    // This pattern has many gliders.
    // Make sure that no gliders get killed when translating...
    LifeState pattern(
        "5bo$6bo8bo3bo12bo$4b3o2bo3bobo4b2o8b2o$10b2o2b2o3b2o10b2o$9b2o2$28bobo"
        "$11b2o15b2o$10bobo16bo$12bo2$5bo27bo$3bobo10b2ob2o12bobo$4b2o10b2ob2o"
        "12b2o3$17b2ob2o$17bo3bo$18bobo$17b2ob2o$5b2o25b2o$4bobo25bobo$6bo12bo"
        "12bo$18bobo$bo17bo17bo$b2o33b2o$obo33bobo2$9b2o17b2o$10b2o15b2o$9bo19b"
        "o$22b2o$22bobo$22bo2$16b2o$15bobo$17bo!",
        -20, -10
    );
    LifeState temp("2bo$2o$b2o!", -2, -17);
    LifeState g[4];
    for (int i=0; i<4; ++i)
    {
        g[i] = temp;
        temp.run();
    }
    CellList rect_g = LifeState::makeRect(-20, -20, 50, 50).toCellList();
    for(CellList::iterator it=rect_g.begin(); it != rect_g.end(); ++it)
    {
        for (int i=0; i<4; ++i)
        {
            LifeState state(pattern);
            state |= g[i].transform(it->x, it->y);
            LifeState backup(state);
            state.run(45);
            bool is_dart = true;
            for (int j=0; j<10; ++j)
            {
                if (state.getPop() != 40)
                {
                    is_dart = false;
                    break;
                }
                state.run();
                if (state.getPop() != 34)
                {
                    is_dart = false;
                    break;
                }
                state.run(2);
            }
            if (is_dart)
            {
                // std::cerr << backup.toRLE() << std::endl;
                return true;
            }
        }
    }
    return false;
}

// Test 03. Add two gliders to finish the bi-snake synth.
bool testSimkin03()
{
    int num_solutions = 0;
	LifeState pattern = LifeState(
        "obo$b2o$bo9$4bo$4b2o$3bobo$7b3o$7bo$8bo$14bo$13b2o$13bobo!", -20, -20
    );
	LifeState target_on = LifeState(
        "$b2ob2o$bo3bo$2bobo$b2ob2o3$3bo$2bobo$3bo!", -18, -10
    );
    // We don't need inverse patterns anymore.
	// LifeState inverse = LifeState("7o$o2bo2bo$ob3obo$2obob2o$o2bo2bo$7o$7o$3ob3o$2obob2o$3ob3o$7o!", -18, -10);
    LifeState target_off = target_on * LifeState::makeRect(-1, -1, 3, 3) - target_on;
    // This needs more work.
    LifeState temp("b2o$obo$2bo!");
    LifeState g[4];
    for (int i=0; i<4; ++i)
    {
        g[i] = temp;
        temp.run();
    }
    CellList range = LifeState::makeRect(-27, 2, 16, 16).toCellList();
    for (CellList::iterator it1 = range.begin(); it1 != range.end(); ++it1)
    {
        for (CellList::iterator it2 = range.begin(); it2 != range.end(); ++it2)
        {
            for (int i1=0; i1<4; ++i1)
            {
                for(int i2=0; i2<4; ++i2)
                {
                    LifeState gliders = (g[i1].transform(it1->x, it1->y)) | (g[i2].transform(it2->x, it2->y));
                    // Make sure the gliders don't interact.
                    if (gliders.after(4) != gliders.transform(1, -1))
                    {
                        continue;
                    }
                    LifeState state = pattern | gliders;
                    LifeState backup = state;
                    state.run(60);
                    if (state.contains(target_on) && (~state).contains(target_off))
                    {
                        // The solution should be:
                        // 5bobo$6b2o$6bo9$9bo$9b2o$8bobo$12b3o$12bo$13bo$19bo$18b2o$18bobo6$9b2o$8bobo$10bo2$2o$b2o$o!
                        // std::cerr << backup.toRLE() << std::endl;
                        num_solutions++;
                    }
                }
            }
        }
    }
    // It should return 2C1 copies of the same solution.
    return (num_solutions == 2);
}

int main(void)
{
    testWithMsg(testInit01, "LifeState init test 01");
    testWithMsg(testInit02, "LifeState init test 02");
    testWithMsg(testInitRLE01, "LifeState init test with RLE 01 - With getCell");
    testWithMsg(testInitRLE02, "LifeState init test with RLE 02 - With evolution");
    testWithMsg(testInitRLE03, "LifeState init test with RLE 03 - Without count numbers");
    testWithMsg(testInitRLE04, "LifeState init test with RLE 04 - Without footer '!'");
    testWithMsg(testOpAnd01, "Operator &= test");
    testWithMsg(testOpAnd02, "Operator & test");
    testWithMsg(testOpOr01, "Operator |= test");
    testWithMsg(testOpOr02, "Operator | test");
    testWithMsg(testOpXor01, "Operator ^= test");
    testWithMsg(testOpXor02, "Operator ^ test");
    testWithMsg(testOpPlusMinus, "Operator +, - test");
    testWithMsg(testTransform01, "LifeState transformation test 01");
    testWithMsg(testTransform02, "LifeState transformation test 02");
    testWithMsg(testLifeLocator01, "LifeLocator basic test - Wanted Cells");
    testWithMsg(testLifeLocator02, "LifeLocator basic test - Unwanted Cells");
    testWithMsg(testRemoveGliders01, "Glider Removal test 01");
    testWithMsg(testPatternMatching01, "Pattern matching basic test 01 - Locating");
    testWithMsg(testPatternMatching02, "Pattern matching basic test 02 - Removal");
    testWithMsg(testSimkin01, "Advanced test from Michael Simkin #01 - Glider collisions");
    testWithMsg(testSimkin02, "Advanced test from Michael Simkin #02 - Dart synthesis");
    testWithMsg(testSimkin03, "Advanced test from Michael Simkin #03 - Bi-snake synthesis");
    return 0;
}