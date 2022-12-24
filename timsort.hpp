#include <vector>
#include <algorithm>
#include <functional>
#include <iterator>
#include <utility>

struct run {
    typedef int* Iterator;
    typedef std::ptrdiff_t diff_t;

    Iterator base;
    diff_t len;

    run(Iterator b, diff_t l) : base(b), len(l) {
    }
};

template<typename Compare = std::less<int>> class TimSort {
    typedef int& ref_t;
    typedef std::ptrdiff_t diff_t;
    typedef int* iter_t;

    static constexpr int MIN_MERGE = 32;
    static constexpr int MIN_GALLOP = 7;

    int minGallop_; // default to MIN_GALLOP

    std::vector<int> tmp_; // temp storage for merges
    typedef typename std::vector<int>::iterator tmp_iter_t;

    std::vector<run> pending_;

    TimSort() : minGallop_(MIN_GALLOP) {
    }

    // Silence GCC -Winline warning
    ~TimSort() {}

    static void binarySort(iter_t const lo, iter_t const hi, iter_t start, Compare compare) {
        
        if (start == lo) {
            ++start;
        }
        for (; start < hi; ++start) {
            
            int pivot = std::move(*start);

            iter_t const pos = std::upper_bound(lo, start, pivot, compare);
            for (iter_t p = start; p > pos; --p) {
                *p = std::move(*std::prev(p));
            }
            *pos = std::move(pivot);
        }
    }

    static diff_t countRunAndMakeAscending(iter_t const lo, iter_t const hi, Compare compare) {

        auto runHi = std::next(lo);
        if (runHi == hi) {
            return 1;
        }

        if (compare(*runHi, *lo)) { // decreasing
            do {
                ++runHi;
            } while (runHi < hi && compare(*runHi, *std::prev(runHi)));
            std::reverse(lo, runHi);
        } else { // non-decreasing
            do {
                ++runHi;
            } while (runHi < hi && !compare(*runHi, *std::prev(runHi)));
        }

        return runHi - lo;
    }

    static diff_t minRunLength(diff_t n) {        

        diff_t r = 0;
        while (n >= 2 * MIN_MERGE) {
            r |= (n & 1);
            n >>= 1;
        }
        return n + r;
    }

    void pushRun(iter_t const runBase, diff_t const runLen) {
        pending_.emplace_back(runBase, runLen);
    }

    void mergeCollapse(Compare compare) {
        while (pending_.size() > 1) {
            diff_t n = pending_.size() - 2;

            if ((n > 0 && pending_[n - 1].len <= pending_[n].len + pending_[n + 1].len) ||
                (n > 1 && pending_[n - 2].len <= pending_[n - 1].len + pending_[n].len)) {
                if (pending_[n - 1].len < pending_[n + 1].len) {
                    --n;
                }
                mergeAt(n, compare);
            } else if (pending_[n].len <= pending_[n + 1].len) {
                mergeAt(n, compare);
            } else {
                break;
            }
        }
    }

    void mergeForceCollapse(Compare compare) {
        while (pending_.size() > 1) {
            diff_t n = pending_.size() - 2;

            if (n > 0 && pending_[n - 1].len < pending_[n + 1].len) {
                --n;
            }
            mergeAt(n, compare);
        }
    }

    void mergeAt(diff_t const i, Compare compare) {
        diff_t const stackSize = pending_.size();
        iter_t base1 = pending_[i].base;
        diff_t len1 = pending_[i].len;
        iter_t base2 = pending_[i + 1].base;
        diff_t len2 = pending_[i + 1].len;

        pending_[i].len = len1 + len2;

        if (i == stackSize - 3) {
            pending_[i + 1] = pending_[i + 2];
        }

        pending_.pop_back();

        mergeConsecutiveRuns(base1, len1, base2, len2, std::move(compare));
    }

    void mergeConsecutiveRuns(iter_t base1, diff_t len1, iter_t base2, diff_t len2, Compare compare) {

        diff_t const k = gallopRight(*base2, base1, len1, 0, compare);

        base1 += k;
        len1 -= k;

        if (len1 == 0) {
            return;
        }

        len2 = gallopLeft(*(base1 + (len1 - 1)), base2, len2, len2 - 1, compare);
        
        if (len2 == 0) {
            return;
        }

        if (len1 <= len2) {
            mergeLo(base1, len1, base2, len2, compare);
        } else {
            mergeHi(base1, len1, base2, len2, compare);
        }
    }

    template <typename Iter>
    static diff_t gallopLeft(ref_t key, Iter const base, diff_t const len,
                             diff_t const hint, Compare compare) {

        diff_t lastOfs = 0;
        diff_t ofs = 1;

        if (compare(*(base + hint), key)) {
            diff_t const maxOfs = len - hint;
            while (ofs < maxOfs && compare(*(base + (hint + ofs)), key)) {
                lastOfs = ofs;
                ofs = (ofs << 1) + 1;

                if (ofs <= 0) { // int overflow
                    ofs = maxOfs;
                }
            }
            if (ofs > maxOfs) {
                ofs = maxOfs;
            }

            lastOfs += hint;
            ofs += hint;
        } else {
            diff_t const maxOfs = hint + 1;
            while (ofs < maxOfs && !compare(*(base + (hint - ofs)), key)) {
                lastOfs = ofs;
                ofs = (ofs << 1) + 1;

                if (ofs <= 0) {
                    ofs = maxOfs;
                }
            }
            if (ofs > maxOfs) {
                ofs = maxOfs;
            }

            diff_t const tmp = lastOfs;
            lastOfs = hint - ofs;
            ofs = hint - tmp;
        }

        return std::lower_bound(base + (lastOfs + 1), base + ofs, key, compare) - base;
    }

    template <typename Iter>
    static diff_t gallopRight(ref_t key, Iter const base, diff_t const len,
                              diff_t const hint, Compare compare) {
        diff_t ofs = 1;
        diff_t lastOfs = 0;

        if (compare(key, *(base + hint))) {
            diff_t const maxOfs = hint + 1;
            while (ofs < maxOfs && compare(key, *(base + (hint - ofs)))) {
                lastOfs = ofs;
                ofs = (ofs << 1) + 1;

                if (ofs <= 0) {
                    ofs = maxOfs;
                }
            }
            if (ofs > maxOfs) {
                ofs = maxOfs;
            }

            diff_t const tmp = lastOfs;
            lastOfs = hint - ofs;
            ofs = hint - tmp;
        } else {
            diff_t const maxOfs = len - hint;
            while (ofs < maxOfs && !compare(key, *(base + (hint + ofs)))) {
                lastOfs = ofs;
                ofs = (ofs << 1) + 1;

                if (ofs <= 0) { // int overflow
                    ofs = maxOfs;
                }
            }
            if (ofs > maxOfs) {
                ofs = maxOfs;
            }

            lastOfs += hint;
            ofs += hint;
        }

        return std::upper_bound(base + (lastOfs + 1), base + ofs, key, compare) - base;
    }

    static void rotateLeft(iter_t first, iter_t last)
    {
        int tmp = std::move(*first);
        auto last_1 = std::move(std::next(first), last, first);
        *last_1 = std::move(tmp);
    }

    static void rotateRight(iter_t first, iter_t last)
    {
        auto last_1 = std::prev(last);
        int tmp = std::move(*last_1);
        std::move_backward(first, last_1, last);
        *first = std::move(tmp);
    }


    void mergeLo(iter_t const base1, diff_t len1, iter_t const base2, diff_t len2, Compare compare) {
        if (len1 == 1) {
            return rotateLeft(base1, base2 + len2);
        }
        if (len2 == 1) {
            return rotateRight(base1, base2 + len2);
        }

        copy_to_tmp(base1, len1);

        tmp_iter_t cursor1 = tmp_.begin();
        iter_t cursor2 = base2;
        iter_t dest = base1;

        *dest = std::move(*cursor2);
        ++cursor2;
        ++dest;
        --len2;

        int minGallop(minGallop_);

        // outer:
        while (true) {
            diff_t count1 = 0;
            diff_t count2 = 0;

            do {
                if (compare(*cursor2, *cursor1)) {
                    *dest = std::move(*cursor2);
                    ++cursor2;
                    ++dest;
                    ++count2;
                    count1 = 0;
                    if (--len2 == 0) {
                        goto epilogue;
                    }
                } else {
                    *dest = std::move(*cursor1);
                    ++cursor1;
                    ++dest;
                    ++count1;
                    count2 = 0;
                    if (--len1 == 1) {
                        goto epilogue;
                    }
                }
            } while ((count1 | count2) < minGallop);

            do {
                count1 = gallopRight(*cursor2, cursor1, len1, 0, compare);
                if (count1 != 0) {
                    std::move_backward(cursor1, cursor1 + count1, dest + count1);
                    dest += count1;
                    cursor1 += count1;
                    len1 -= count1;

                    if (len1 <= 1) {
                        goto epilogue;
                    }
                }
                *dest = std::move(*cursor2);
                ++cursor2;
                ++dest;
                if (--len2 == 0) {
                    goto epilogue;
                }

                count2 = gallopLeft(*cursor1, cursor2, len2, 0, compare);
                if (count2 != 0) {
                    std::move(cursor2, cursor2 + count2, dest);
                    dest += count2;
                    cursor2 += count2;
                    len2 -= count2;
                    if (len2 == 0) {
                        goto epilogue;
                    }
                }
                *dest = std::move(*cursor1);
                ++cursor1;
                ++dest;
                if (--len1 == 1) {
                    goto epilogue;
                }

                --minGallop;
            } while ((count1 >= MIN_GALLOP) | (count2 >= MIN_GALLOP));

            if (minGallop < 0) {
                minGallop = 0;
            }
            minGallop += 2;
        } // end of "outer" loop

        epilogue: // merge what is left from either cursor1 or cursor2

        minGallop_ = (std::min)(minGallop, 1);

        if (len1 == 1) {
            std::move(cursor2, cursor2 + len2, dest);
            *(dest + len2) = std::move(*cursor1);
        } else {
            std::move(cursor1, cursor1 + len1, dest);
        }
    }

    void mergeHi(iter_t const base1, diff_t len1, iter_t const base2, diff_t len2, Compare compare) {
        if (len1 == 1) {
            return rotateLeft(base1, base2 + len2);
        }
        if (len2 == 1) {
            return rotateRight(base1, base2 + len2);
        }

        copy_to_tmp(base2, len2);

        iter_t cursor1 = base1 + len1;
        tmp_iter_t cursor2 = tmp_.begin() + (len2 - 1);
        iter_t dest = base2 + (len2 - 1);

        *dest = std::move(*(--cursor1));
        --dest;
        --len1;

        int minGallop(minGallop_);

        // outer:
        while (true) {
            diff_t count1 = 0;
            diff_t count2 = 0;

            // The next loop is a hot path of the algorithm, so we decrement
            // eagerly the cursor so that it always points directly to the value
            // to compare, but we have to implement some trickier logic to make
            // sure that it points to the next value again by the end of said loop
            --cursor1;

            do {
                if (compare(*cursor2, *cursor1)) {
                    *dest = std::move(*cursor1);
                    --dest;
                    ++count1;
                    count2 = 0;
                    if (--len1 == 0) {
                        goto epilogue;
                    }
                    --cursor1;
                } else {
                    *dest = std::move(*cursor2);
                    --cursor2;
                    --dest;
                    ++count2;
                    count1 = 0;
                    if (--len2 == 1) {
                        ++cursor1; // See comment before the loop
                        goto epilogue;
                    }
                }
            } while ((count1 | count2) < minGallop);
            ++cursor1; // See comment before the loop

            do {
                count1 = len1 - gallopRight(*cursor2, base1, len1, len1 - 1, compare);
                if (count1 != 0) {
                    dest -= count1;
                    cursor1 -= count1;
                    len1 -= count1;
                    std::move_backward(cursor1, cursor1 + count1, dest + (1 + count1));

                    if (len1 == 0) {
                        goto epilogue;
                    }
                }
                *dest = std::move(*cursor2);
                --cursor2;
                --dest;
                if (--len2 == 1) {
                    goto epilogue;
                }

                count2 = len2 - gallopLeft(*std::prev(cursor1), tmp_.begin(), len2, len2 - 1, compare);
                if (count2 != 0) {
                    dest -= count2;
                    cursor2 -= count2;
                    len2 -= count2;
                    std::move(std::next(cursor2), cursor2 + (1 + count2), std::next(dest));
                    if (len2 <= 1) {
                        goto epilogue;
                    }
                }
                *dest = std::move(*(--cursor1));
                --dest;
                if (--len1 == 0) {
                    goto epilogue;
                }

                --minGallop;
            } while ((count1 >= MIN_GALLOP) | (count2 >= MIN_GALLOP));

            if (minGallop < 0) {
                minGallop = 0;
            }
            minGallop += 2;
        } // end of "outer" loop

        epilogue: // merge what is left from either cursor1 or cursor2

        minGallop_ = (std::min)(minGallop, 1);

        if (len2 == 1) {
            dest -= len1;
            std::move_backward(cursor1 - len1, cursor1, dest + (1 + len1));
            *dest = std::move(*cursor2);
        } else {
            std::move(tmp_.begin(), tmp_.begin() + len2, dest - (len2 - 1));
        }
    }

    void copy_to_tmp(iter_t const begin, diff_t len) {
        tmp_.assign(std::make_move_iterator(begin),
                    std::make_move_iterator(begin + len));
    }

public:

    static void merge(iter_t const lo, iter_t const mid, iter_t const hi, Compare compare = std::less<int>{}) {
        if (lo == mid || mid == hi) {
            return; // nothing to do
        }

        TimSort ts;
        ts.mergeConsecutiveRuns(lo, mid - lo, mid, hi - mid, std::move(compare));
    }

    static void sort(iter_t const lo, iter_t const hi, int minRun = 0, Compare compare = std::less<int>{}) {
        diff_t nRemaining = (hi - lo);
        if (nRemaining < 2) {
            return; // nothing to do
        }

        if (nRemaining < MIN_MERGE) {
            diff_t const initRunLen = countRunAndMakeAscending(lo, hi, compare);
            
            binarySort(lo, hi, lo + initRunLen, compare);
            return;
        }

        TimSort ts;

        if(minRun == 0){
            minRun = minRunLength(nRemaining);
        }

        iter_t cur = lo;
        do {
            diff_t runLen = countRunAndMakeAscending(cur, hi, compare);

            if (runLen < minRun) {
                diff_t const force = (std::min)(nRemaining, minRun);
                binarySort(cur, cur + force, cur + runLen, compare);
                runLen = force;
            }

            ts.pushRun(cur, runLen);
            ts.mergeCollapse(compare);

            cur += runLen;
            nRemaining -= runLen;
        } while (nRemaining != 0);
        ts.mergeForceCollapse(compare);
    }
};
