
#include "units/TestPatternSelection.hpp"

#include "core/PatternSelection.hpp"

#include <array>
#include <string>

Q_DECLARE_METATYPE(PatternSelection)

// might want to move these to the ui library

bool operator==(PatternSelection::Iterator const& lhs, PatternSelection::Iterator const& rhs) {
    // check that the starting and ending anchors are equal
    return lhs.start() == rhs.start() && lhs.end() == rhs.end();
}

bool operator==(PatternSelection const& lhs, PatternSelection const& rhs) {
    // selections are equivalent if their iterators are equivalent
    return lhs.iterator() == rhs.iterator();
}

TestPatternSelection::TestPatternSelection()
{
}

void TestPatternSelection::normalization_data() {

    // this test checks that PatternSelections are equivalent by
    // normalizing the selection range.


    // anchor position for each corner of the selection rectangle
    constexpr PatternAnchor TOP_LEFT        {5, 1, 0};
    constexpr PatternAnchor TOP_RIGHT       {5, 4, 1};
    constexpr PatternAnchor BOTTOM_RIGHT    {9, 4, 1};
    constexpr PatternAnchor BOTTOM_LEFT     {9, 1, 0};

    QTest::addColumn<PatternSelection>("selection1");
    QTest::addColumn<PatternSelection>("selection2");


    // all possible selections using each corner pairs
    std::array const data = {
        PatternSelection(TOP_LEFT, BOTTOM_RIGHT),
        PatternSelection(BOTTOM_RIGHT, TOP_LEFT),
        PatternSelection(TOP_RIGHT, BOTTOM_LEFT),
        PatternSelection(BOTTOM_LEFT, TOP_RIGHT)
    };

    std::array const names = {
        "(TL, BR)",
        "(BR, TL)",
        "(TR, BL)",
        "(BL, TR)"
    };


    auto rowName = [&names](size_t left, size_t right) {
        std::string name;
        name += names[left];
        name += " == ";
        name += names[right];
        return name;
    };

    // identity
    for (size_t i = 0; i < data.size(); ++i) {
        // ensure that each selection is equal to itself
        QTest::newRow(rowName(i, i).c_str()) << data[i] << data[i];
    }

    // permutations
    // check that every selection is equalivalent
    for (size_t left = 0; left < data.size() - 1; ++left) {
        for (size_t right = left + 1; right < data.size(); ++right) {
            QTest::newRow(rowName(left, right).c_str()) << data[left] << data[right];
        }
    }

}

void TestPatternSelection::normalization() {
    QFETCH(PatternSelection, selection1);
    QFETCH(PatternSelection, selection2);

    QCOMPARE(selection1, selection2);
}

void TestPatternSelection::translate_data() {
    QTest::addColumn<PatternSelection>("input");
    QTest::addColumn<int>("rows");
    QTest::addColumn<PatternSelection>("output");

    constexpr int TRANSPOSE1 = 4;
    constexpr int TRANSPOSE2 = -3;

    constexpr PatternAnchor START(0, 1, 0);
    constexpr PatternAnchor END(5, 4, 2);


    PatternSelection input(START, END);

    QTest::newRow("0") << input << 0 << input;


    QTest::newRow("4") << input
                       << TRANSPOSE1
                       << PatternSelection(
                               PatternAnchor(START.row + TRANSPOSE1, START.column, START.track),
                               PatternAnchor(END.row + TRANSPOSE1, END.column, END.track)
                               );
    QTest::newRow("-3") << input
                        << TRANSPOSE2
                        << PatternSelection(
                               PatternAnchor(START.row + TRANSPOSE2, START.column, START.track),
                               PatternAnchor(END.row + TRANSPOSE2, END.column, END.track)
                               );
}


void TestPatternSelection::translate() {
    QFETCH(PatternSelection, input);
    QFETCH(int, rows);
    QFETCH(PatternSelection, output);

    input.translate(rows);
    QCOMPARE(input, output);
}

void TestPatternSelection::iteration_data() {
    QTest::addColumn<PatternSelection>("input");
    QTest::addColumn<int>("rowsIterated");
    QTest::addColumn<int>("columnsIterated");
    QTest::addColumn<int>("tracksIterated");

    QTest::addRow("empty")
            << PatternSelection() << 1 << 1 << 1;

    QTest::addRow("single column")
            << PatternSelection({1, 0, 0}, {6, 0, 0}) << 6 << 1 << 1;

    QTest::addRow("multiple columns, single track")
            << PatternSelection({10, 1, 3}, {1, 4, 3}) << 10 << 4 << 1;

    QTest::addRow("single column, multiple tracks")
            << PatternSelection({3, 3, 1}, {3, 3, 3}) << 1 << 11 << 3;

    QTest::addRow("multiple columns, multiple tracks")
            << PatternSelection({1, 4, 3}, {10, 1, 1}) << 10 << 14 << 3;

}

void TestPatternSelection::iteration() {
    QFETCH(PatternSelection, input);
    QFETCH(int, rowsIterated);
    QFETCH(int, columnsIterated);
    QFETCH(int, tracksIterated);

    auto const iter = input.iterator();
    int trackCount = 0;
    int rowCount = iter.rowEnd() - iter.rowStart() + 1;
    int columnCount = 0;

    for (auto track = iter.trackStart(); track <= iter.trackEnd(); ++track) {
        auto const tmeta = iter.getTrackMeta(track);
        columnCount += tmeta.columnEnd() - tmeta.columnStart() + 1;


        ++trackCount;
    }

    QCOMPARE(rowCount, rowsIterated);
    QCOMPARE(columnCount, columnsIterated);
    QCOMPARE(trackCount, tracksIterated);
}
