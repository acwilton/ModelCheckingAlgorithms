# ModelCheckingAlgorithms
My implementation of a basic model checking library in C++17 using the algorithms and concepts from the Model Checking book. Particularly Chapter 7 of the book.

Algorithms implemented:
 + Buchi intersection of two Buchi automata
 + Double DFS to find accepting run of a Buchi
 + Fair Kripke to Buchi algorithm
 + LTL normalization algorithm (converts to negation normal form and replaces future and global subformulas to their equivalent untile and release subformulas)
 + LTL to Buchi algorithm
 + A model checking algorithm that combines all of the above to determine if there exists a run on a Kripke structure that satisfies an LTL formula, and returns a lasso if one exists

There are two drivers that demonstrate the capabilities of the library. To build them simply run `make` under the `src/` folder. They will compile into the files `buchi_driver` and `int_kripke_driver`.

# Buchi Driver
The buchi driver will parse a file that specifies two buchi automata and will then take their intersection and try to find an accepting run. If one is found, it will print the lasso it forms.

It can be run with a filename as input like so: `./buchi_driver example.buchi`. The filename should be of a file that holds the specification of two Buchi automaton. 

The specification of a Buchi automaton looks as follows:
```
{
  <init1, init2, ..., initN>
  [acc1, acc2, ..., accM]
  (a1, l1, b1)
  (a2, l2, b2)
  ...
  (aK, lK, bK)
}
```
The dots `...` are not part of the syntax. They are just there to demonstrate a variable number of things.
The states and labels of the Buchi automata specified are strings. Two states are the same if they are the same string (ignoring trailing and leading whitespace). The same goes for labels.
The angle brackets `<init1, init2, ..., initN>` holds the list of starting states.
The square brackets `[acc1, acc2, ..., accM]` holds the list of accepting states.
A triple `(ai, li, bi)` represents a labeled transition from node `ai` to node `bi` with the transition labeled by the string `li`. The states of the system are implicitly defined by the appearance of a state in some transition, or in the list of starting states or the list of accepting states.

For the driver to accept a buchi specification file, it must have exactly two buchi structures as written above (whitespacing differences are ignored).

To help get started playing with this tool, there is an example file you can run the driver on called `example.buchi`.

# Int Kripke Driver
The int kripke driver will parse a file that specifies an LTL-x (LTL without the X operator) formula and a fair Kripke structure, in which the states are integers and the atomic propositions are boolean functions on integers. It will then determine if the LTL-x formula holds on all fair paths, printing a lasso counterexample if it does not hold.

It can be run with a filename and optional positive integer like so: `./int_kripke_driver collatz1.kripke 12`. If an integer is not given, it defaults to 1000. The filename should be of a file that holds the specification of an LTL formula and Kripke structure on integers. The optional integer argument, N, which we will call the "cap", acts as exactly that. It caps the integers defined in the transitions by taking each integer transitioned to modulo N. This is to ensure finiteness of the Kripke structure.

The file passed into the kripke driver must always start with the LTL-x specification, which begins with `spec = ` followed by an LTL-x formula. The specification of an LTL-x formula can be done with the following grammar:
```
FORMULA -> ( G FORMULA )
        -> ( F FORMULA )
        -> ( U FORMULA FORMULA )
        -> ( R FORMULA FORMULA )
        -> ( ! FORMULA )
        -> ( && FORMULA FORMULA )
        -> ( || FORMULA FORMULA )
        -> AP

AP      -> ( == EXPR EXPR )
        -> ( =/= EXPR EXPR )
        -> ( >= EXPR EXPR )
        -> ( > EXPR EXPR )
        -> ( <= EXPR EXPR )
        -> ( < EXPR EXPR )

EXPR    -> ( + EXPR EXPR )
        -> ( - EXPR EXPR )
        -> ( * EXPR EXPR )
        -> ( / EXPR EXPR )
        -> ( % EXPR EXPR )
        -> s
        -> N
        -> INTEGER
```
Where INTEGER is an actual integer. The `s` is the parameter standing for the current state. The `N` stands for the cap passed in from the command line. Each of the other symbols have the obvious meaning (although to be safe, I will clarify that `=/=` means not equal, `% EXPR1 EXPR1` means `EXPR1` modulo `EXPR2`, and `/` is integer division).
For example:
```
(! (F (G (&&
            (== (% s 2) 0)
            (=/= (% s 7) 0)))))
```
Says "It is not the case that in the future we will globally have even integers not divisible by 7."

After the specification is written, we must define the kripke structure that the LTL specification will be checked against. The definition of a kripke structure looks as follows:
```
init = <init1, init2, ..., initN>
fair = [{fair1_1, fair1_2, ..., fair1_M1},{fair2_1, fair2_2, ..., fair2_M2}, ..., {fairK_1, fairK_2, ..., fairK_MK}]
{ from1_1, from1_2, ..., from1_F1 } -> { to1_1, to1_2, ..., to1_T1 }
{ from2_1, from2_2, ..., from2_F2 } -> { to2_1, to2_2, ..., to2_T2 }
...
{ fromL_1, fromL_2, ..., fromL_FL } -> { toL_1, toL_2, ..., toL_TL }
```
In the above, any list (a sequence which has `...` displayed) may be empty if desired (although it may make the kripke structure very boring). For instance, a kripke structure in which all paths are fair may have no fairness sets at all and thus one would write `fair = []`.

Each of `initi` in the init list `init = <...>` is simply an integer. These define the set of integers that act as the initial states of the kripke strucutre. Note that these integers are NOT taken modulo the passed in cap N.

Each of `fairi_j` are a "characteristic function" as defined by the following grammar:
```
CHAR_FUNC -> ( == EXPR EXPR )
          -> ( =/= EXPR EXPR )
          -> ( >= EXPR EXPR )
          -> ( > EXPR EXPR )
          -> ( <= EXPR EXPR )
          -> ( < EXPR EXPR )
          -> INTEGER

EXPR      -> ( + EXPR EXPR )
          -> ( - EXPR EXPR )
          -> ( * EXPR EXPR )
          -> ( / EXPR EXPR )
          -> ( % EXPR EXPR )
          -> s
          -> N
          -> INTEGER
```
The semantics of a `CHAR_FUNC`, `F`, is such that an integer s "intersects" with `F`, or "satisfies" `F`, if either `F` is an integer v and v == s, or s satisfies the boolean function defined by F in the grammar. So for instance
```
fair = [{(== (% s 2) 0)}, {(== (% s 3) 1), 0, 5}]
```
says that a path is fair iff it reaches an even integer an infinite number of times, and if it reaches either 0, 5, or an integer s such that s % 3 == 1, an infinte number of times.

Each of `fromi_j` are also "characterstic functions" as defined by the grammar of `CHAR_FUNC` above. Each of `toi_j` are an arithmetic expression as defined by the subgrammar `EXPR` found in both of the previously defined grammars.
The semantics of the list of transitions
```
{ from1_1, from1_2, ..., from1_F1 } -> { to1_1, to1_2, ..., to1_T1 }
{ from2_1, from2_2, ..., from2_F2 } -> { to2_1, to2_2, ..., to2_T2 }
...
{ fromL_1, fromL_2, ..., fromL_FL } -> { toL_1, toL_2, ..., toL_TL }
```
is such that an integer `f` transitions to integer `t` if there is a `fromi_j` and a `toi_k` such that `f` satisfies `fromi_j`, and `t == toi_k(f) % N`, where `toi_k(f)` is the result of plugging `f` into the arithmetic function defined by the grammar of `EXPR`. Note that this is where the cap `N` comes in. We take modulo `N` of the "heads" of every transition so that we maintain a finite structure. Note that this modulo operator, and the one definied for arithmetic expressions, is using the c++ % operator which will actually return negative values when the left operand is negative.

Another way of thinking about the transition lists is that if an integer `s` satisfies any of `{ fromi_1, fromi_2, ..., fromi_Fi }`, then there is a transition from `s` to each of `toi_1(s) % N, toi_2(s) % N, ..., toi_Ti(s) % N`.

To get started playing with this driver, there are four examples committed. `collatz1.kripke` and `collatz2.kripke` both define the same Kripke structure, which is the reverse collatz graph. The other two examples are `example1.kripke` and `example2.kripke` and are somewhat arbitrary and are mostly there as examples on how to define different sorts of kripke structures.
