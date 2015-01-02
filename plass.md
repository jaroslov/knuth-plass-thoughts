Errata
------

Please see:

    https://bugzilla.mozilla.org/show_bug.cgi?id=630181#c8

A copy of this comment is placed at the end of this discussion.

Introduction
------------

Line-breaking algorithms take a paragraph's-worth of words, and split the words into line-lengthed chunks. The two algorithms many programmer's know of are:

1. The greedy algorithm; and,
2. The Knuth-Plass algorithm (the 'latex one').

Most programmer's "know" the following three facts:

1. Knuth-Plass produces the 'best' line breaks;
2. Knuth-Plass is a quadratic algorihtm; and,
3. Knuth-Plass uses dynamic programming and is impossible for mere mortals to code.

While we happen to agree with (1), we will demonstrate that (2) and (3) are, respectively, not true, and unnecessarily obscure.
In fact, the Knuth-Plass algorithm---even in its most naive implementation---is strongly dominated by a light-weight linear run-time, and the implementation of the core algorithm is remarkably straightforward.

The rest of this article is organized:

1. What is line-breaking;
2. Knuth & Plass' intuition about scoring;
3. The Knuth-Plass algorithm; and,
4. Further optimizations & features.

What is Line Breaking
---------------------

We consider the simpler case of line-breaking fixed-width text. Line-breaking proportional fonts (with glue) is a matter of more bookkeeping. Our goal, then, is given a list of words and a line-length, to split those words into separate lines. We will only consider line-breaking of rectangular paragraphs. It is usually natural to try to have the first lines all be as long as possible, with a last line that is potentially much shorter: the greedy algorithm. An implementation could be:

    INPUT: length, words
    OUTPUT: breaks

    currentlength       := 0
    breaks              := []
    for wdx,word in enumerate(words):
        if len(word) + currentlength >= length:
            breaks.append(wdx)
            currentlength := 0
        currentlength   += len(word) + 1 ;; account for space

A number of features can be added to this simple line-breaking algorithm, for instance, non-rectangular shapes. To add this feature, we take, as input, a list of line-lengths. we then break each line according to its individual line-length.

    INPUT: lengths, words
    OUTPUT: breaks

    currentlength       := 0
    breaks              := []
    currentline         := 0
    for wdx,word in enumerate(words):
        if len(word) + currentlength >= lengths[currentline]:
            breaks.append(wdx)
            currentlength := 0
            currentline += 1
        currentlength   += len(word) + 1 ;; account for space

Knuth & Plass' intuition about scoring
--------------------------------------

One thing that is not evident in the greedy algorithm is that we are implicitly defining a score for how good each line break is; and, a metric for how good the total set of line breaks we've chosen are, together. Knuth & Plass' intuition was to explicitly define the scoring system (and the metric), and then choose a scoring system and metric that resulted in both a computationally tractable algorithm, and one that gives high quality results.

In pseudocode the greedy line-breaking algorithm's score is defined as:

    score ==    if cur-line-length < max-line-length:
                    max-line-length - cur-line-length
                else:
                    infinity

Instead, the Knuth & Plass line-breaking algorithm defines an ideal line length, and a maximum line length, and give a score accordingly:

    score ==    if cur-line-length < max-line-length:
                                                         2
                    (ideal-line-length - cur-line-length)
                else:
                    infinity

The ability to choose lines that are both too long and too short gives the Knuth & Plass line-breaking algorithm more flexibility in choosing line-breaks; as such, it sets the stage to allow the Knuth & Plass algorithm to find a 'more rectangular' set of line-breaks.

A small picture will make this intuition somewhat clearer. In greedy line-breaking algorithm the score is the distance from the right-most word to the maximum word-length column. Should the right-most word cross the maximum word-length column, the score is infinite.

    xxxx x xxxx xxxxx xx xx xxxxx x |
    xx xxxxxxx xxxxx xxx xxxx xxx   |
    xxxxx x x xxx xxxx xxx xx xx    |
    xxxxxxx x xxxx xx xxxx xxx xxx  |
    xx

Knuth & Plass then define a 'metric'---a fancy way to saying how to decide which paragraph is 'best' by saying that, given a scoring system, we want to choose the lowest score. For instance, the above greedily split paragraph might look like as follows, instead (where the ':' mark the ideal width):

    xxxx x xxxx xxxxx xx xx xxxxx : |
    x xx xxxxxxx xxxxx xxx xxxx xxx |
    xxxxx x x xxx xxxx xxx xx xx  : |
    xxxxxxx x xxxx xx xxxx xxx xxx: |
    xx

Which is a far more 'square' paragraph than the greedy paragraph.

The Knuth-Plass algorithm
-------------------------

Given that we have a scoring system (and a metric: pick the lowest score), the algorithm for finding the best set of line-breaks is to:

1. Enumerate all the possible line-breaks;
2. Score all the line-breaks; and,
3. Pick the lowest-scoring set of line-breaks.

Unfortunately, such an approach leads to a rather long list. Conservatively, if we have w words, then there are:

     w
    2

Possible sets of line-breaks---even for fairly modest paragraphs, such as our examples above---there would be something on the order of a billion possible line-breaks. A more vigorous approach to removing uninteresting line-breaking solutions is called for.

Knuth & Plass' next intuition can be summarized as "dynamic programming". This is a particularly irritating term that has no meaning---something that they admit to. Basically, in the Knuth-Plass line-breaking algorithm the 'dynamic programming' approach can be summarized as a clever combination of 'depth-first search' and 'a table'. I'll try to explain why these two things, combined together, so dramatically improve the run-time performance of the line-breaking algorithm.

Imagine we have a list of words; we like to think of them as stretching far to the left and right, and we are looking somewhere in the middle:

    ... x xxxxxxx xxxxx xxx xxxx xxx xxxxx x x xxx xxxx xxx xx xx xxxxxxx x xxxx ...
                                          ^

Just to get the ball rolling, I'm going to assert that we want a line to start at the caret (\^). we don't know that this will lead to a good solution of the line-breaking algorithm. Now, let us assume that we can "solve" the line-breaking problem that includes this particular break in the solution. That is, let us pick the lowest-scoring set of line-breaks that includes this particular break. One interesting fact about the solution is that we can solve the left-side and the right-side independently of each other.

Given that we can solve each side independently, we could imagine just setting another caret, arbitrarily, to the right of our first caret:

    ... x xxxxxxx xxxxx xxx xxxx xxx xxxxx x x xxx xxxx ... xx xxx xxxxxxx xxxx xx xxx xxx ...
                                          ^                               ^

Again, the two right-hand-side 'subproblems' can be solved independently of each other. If we keep splitting 'to the right', we'll eventually get to a problem that looks like this:

    ... xxxx xxx xxx xx
       ^

If this remainder (tail) is shorter than the maximum line width, then this is a solution, all by itself. That is, we can just "write down" the solution to this problem. In fact, if we were to move the caret one word "to the right", the solution is still trivial to write down: it is still just the remainder of the tail.

Now, if we were to back up the caret a bit, and find the next 'max-line length' chunk of words:

    ...x xxxxxxx x xxxx xx xxxx xxx xxxx xxx xxx xx
        ^                          ^

Then we have the interesting property that the solution 'to the right' is still the 'best' solution. In fact, for any given position in the list of words, there is only one 'best' solution. Furthermore, if we choose any arbitrary initial set of 'carets', then we'll eventually find a best 'tail'. If we then work our way 'backwards' from the tail, we can find the best 'line plus tail', and the best 'line plus line plus tail', and so forth. Notice that the cost of finding this 'best tail' is always constant in the number of carets---our solution is linear in the number of line-breaks!

There is some nontrivial bookkeeping required to preserve this 'pefect linearity'; in our presentation of the algorithm, we will ignore this bookkeeping and present an algorithm that is dominated by the linearity of the number of line-breaks, but could act quadratic in pathological cases. C++ code follows.

    struct PEntry
    {
        int first;
        int last;
        int next;
        int score;
    };

    void PlassBreak(std::vector<PEntry>& pents, int idx, int idealwidth, int maxwidth)
    {
        int jdx     = idx+1;                                // Look at the following word.
        int llen    = pents[idx].last - pents[idx].first;   // Current line length is the length of the first word, initially.
        int bscore  = idealwidth - llen;                    // The best score is current line length
        bscore      *= bscore;                              // squared.
        int btail   = jdx;                                  // Best tail is current break.

        while ((jdx < pents.size()))                        // Scan down the word list looking for better entries.
        {
            int wwidth  = pents[jdx].last - pents[jdx].first; // Get new potential word
            if ((llen + wwidth) >= maxwidth) break;         // If the total is too long, stop.
            int lscore  = idealwidth - (llen + wwidth);     // Compute a new line length score.
            lscore      *= lscore;
            llen        += wwidth + 1;                      // Add a word and a space ot the current line.

            if (pents[jdx].score == -1)                     // If we haven't solved the subproblem at this
            {                                               // potential line-break, go ahead and do so.
                PlassBreak(pents, jdx, idealwidth, maxwidth);
            }

            if ((lscore + pents[jdx].score) < bscore)       // Is this new score better the current score?
            {
                bscore  = lscore + pents[jdx].score;        // Update to this new score and also
                btail   = jdx;                              // track the new tail.
            }

            ++jdx;                                          // Look at the next word.
        }

        pents[idx].score    = bscore;                       // Write down the best score
        pents[idx].next     = btail;                        // and tail for this subproblem.

        if ((pents[idx].next + 1) == pents.size())          // The last line of the paragraph doesn't
        {                                                   // contribute to the score.
            pents[idx].score    = 0;
        }
    }


Further optimizations & features
--------------------------------

The run-time (complexity) of the presented Knuth-Plass line-breaking algorithm is:

                  2
    O(min(n * w, n ))

For n line-breaks and w words per line (expected).

There are two basic tweaks to the Knuth-Plass algorithm, as shown, that can reduce the algorithmic complexity. The first is to separate the scanning phase from the best subproblem selection phase. That is, we should first scan ahead, look at each additional word, score the word based on its proximity to the ideal line, and order the words by their score:

    linelength      = len(pents[idx].word)
    potentialwords  = []
    while jdx,word in enumerate(pents[idx:]):
        if linelength + len(pents[jdx].word) >= maxwidth: break
        linelength  += len(pents[jdx].word) + 1
        score       = (idealwidth - linelength)^2
        potentialwords.append((score,word))
    potentialwords.sort()

The second tweak is to keep track, globally, of the best score found so far. Combined with the first tweak, the algorithm will only choose 'better' scores, monotonically, and reject 'really bad' subproblems more quickly.

The last change we consider is to add per-line widths. This is done by passing in an array of ideal- and max- line widths into the PlassBreak algorithm, rather than just an ideal- and max- line value.


https://bugzilla.mozilla.org/show_bug.cgi?id=630181#c8
------------------------------------------------------

callcc 2012-02-11 03:10:59 PST
The algorithm can be implemented to run in O(n) time[2,3]. In fact, even the dynamic programming solution will run in O(min(w * n, n^2)) time[5], where w is the maximum number of words on a line. Since w is fixed, the algorithm is linear for large n.

For a true linear time algorithm you have to make some assumptions on the cost function (namely, concavity). This is usually not a problem, but I think that it may be incompatible with varying line widths. There is yet another way to implement the algorithm which runs in O(n log n) time[1], but apparently with smaller constants than the O(n) algorithm[2].

Now, while I don't know the complexities of the CSS float model, I would really appreciate to have some way to render aesthetically pleasing text in a browser. It does not have to support all the bells and whistles of normal HTML text - even a simple text field would be nice.

Paragraph formatting is pretty much a solved problem in academia and I'm posting this to point you or potential implementors at the relevant literature. However, in my opinion, implementing the dynamic programming algorithm with max line cutoff is both easy and fast enough.

References:

The first line of research is into the "least weight subsequence problem", of which Knuth-Plass paragraph formation is a special case. All papers assume that the cost function is concave.

[1] D.S. Hirschberg and L.L. Larmore, The Least Weight Subsequence Problem
(This contains both an O(n log n) time algorithm and an O(n) algorithm with additional assumptions.)

[2] Robert Wilber. 1988. The concave least-weight subsequence problem revisited
(The first O(n) algorithm which works with any concave cost function.)

[3] Z. Galil and K. Park. 1990. A linear-time algorithm for concave one-dimensional dynamic programming
(A simplified O(n) algorithm.)

Both [2] and [3] depend on an algorithm for "monotone matrix search", which is described in:

[4] A Aggarwal, M Klawe, S Moran, P Shor, and R Wilber. 1986. Geometric applications of a matrix searching algorithm

Finally, for the cost function used by TeX you will have to consult the relevant chapter in Knuth's "Digital Typography" book. Additionally, there is a paper/literate program which implements an O(n) time algorithm:

[5] Oege de Moor and Jeremy Gibbons. 1997. Bridging the Algorithm Gap: a Linear-Time Functional Program for Paragraph Formatting
