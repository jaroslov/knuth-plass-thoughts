#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <stack>
#include <string>
#include <vector>

struct PEntry
{
    int first;
    int last;
    int next;
    int score;
};

void PlassBreak(std::vector<PEntry>& lines, int idx, int idealwidth, int maxwidth, int indent=0)
{
    int jdx     = idx+1;
    int llen    = lines[idx].last - lines[idx].first;
    int bscore  = idealwidth - llen;
    bscore      *= bscore;
    int btail   = jdx;

    while ((jdx < lines.size()))
    {
        int wwidth  = lines[jdx].last - lines[jdx].first;
        if ((llen + wwidth) >= maxwidth) break;
        int lscore  = idealwidth - (llen + wwidth);
        lscore      *= lscore;
        llen        += wwidth + 1;

        if (lines[jdx].score == -1)
        {
            PlassBreak(lines, jdx, idealwidth, maxwidth, indent+1);
        }

        if ((lscore + lines[jdx].score) < bscore)
        {
            bscore  = lscore + lines[jdx].score;
            btail   = jdx;
        }

        ++jdx;
    }

    lines[idx].score    = bscore;
    lines[idx].next     = btail;
    if ((lines[idx].next + 1) == lines.size())
    {
        lines[idx].score    = 0;
    }
}

void GreedBreak(std::vector<PEntry>& lines, int idx, int idealwidth, int maxwidth, int indent=0)
{
    int llen    = 0;
    int linest  = 0;

    for (int II = 0, IE = (int)lines.size(); II < IE; ++II)
    {
        int wwidth              = lines[II].last - lines[II].first;
        if ((llen + wwidth + 1) >= idealwidth)
        {
            lines[linest].next  = II-1;
            linest              = II-1;
            llen                = 0;
        }
        llen                    += wwidth + 1;
        assert((llen < maxwidth) && "WTF?");
    }
    lines[linest].next          = lines.size() + 1;
    lines.rbegin()->next        = lines.size() + 1;
}

void PrintPEntry(std::vector<PEntry> const& lines, std::string const& text, int idealwidth, int maxwidth)
{
    int idx     = 0;
    while (idx < lines.size())
    {
        int next    = lines[idx].next;
        std::string line;
        for (int II = idx; (II <= next) && ((II+1) < lines.size()); ++II)
        {
            if ((lines[II].last - lines[II].first) <= 0) break;
            line    += (II == idx) ? "" : " ";
            line    += text.substr(lines[II].first, lines[II].last - lines[II].first);
        }
        if (line.size() < maxwidth)
            line    += std::string(maxwidth - line.size(), ' ');
        line[idealwidth]    = '+';
        line                += '|';
        fprintf(stdout, "%s\n", line.c_str());
        idx     = lines[idx].next;
    }
}

std::vector<PEntry> Break(std::string const& text, int idealwidth, int maxwidth)
{
    std::vector<PEntry> lines;

    int idx = 0;
    while (idx < text.size())
    {
        while ((idx < text.size()) && std::isspace(text[idx]))
        {
            ++idx;
        }
        int start   = idx;
        while ((idx < text.size()) && !std::isspace(text[idx]))
        {
            ++idx;
        }
        if (start < idx)
        {
            lines.push_back({
                .first  = start,
                .last   = idx,
                .next   = -1,
                .score  = -1,
            });
        }
    }
    lines.push_back({
        .first  = -1,
        .last   = -1,
        .next   = -1,
        .score  = 0,
    });

    return lines;
}

int main(int argc, char* argv[])
{

    FILE* fp    = fopen(argv[1], "r");
    if (!fp) return 1;
    fseek(fp, 0, SEEK_END);
    int length  = (int)ftell(fp);
    rewind(fp);

    char buffer[length+1];
    std::memset(&buffer[0], 0, length+1);
    fread(buffer, 1, length, fp);
    fclose(fp);

    std::string text            = buffer;

    int idealwidth              = (int)std::strtoul(argv[2], 0, 0);
    int maxwidth                = (int)std::strtoul(argv[3], 0, 0);
    std::vector<PEntry> lines1  = Break(text, idealwidth, maxwidth);
    std::vector<PEntry> lines2  = lines1;

    GreedBreak(lines2, 0, idealwidth, maxwidth);
    PrintPEntry(lines2, text, idealwidth, maxwidth);

    fprintf(stdout, "%s", "\n");

    PlassBreak(lines2, 0, idealwidth, maxwidth);
    PrintPEntry(lines2, text, idealwidth, maxwidth);

    return 0;
}
