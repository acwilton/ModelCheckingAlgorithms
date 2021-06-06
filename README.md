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

EXPR    -> s
        -> N
        -> INTEGER
        -> ( + EXPR EXPR )
        -> ( - EXPR EXPR )
        -> ( * EXPR EXPR )
        -> ( / EXPR EXPR )
        -> ( % EXPR EXPR )
```
Where INTEGER is an actual integer. The `s` is the parameter standing for the current state. The `N` stands for the cap passed in from the command line. Each of the other symbols have the obvious meaning (although to be safe, I will clarify that `=/=` means not equal, `% EXPR1 EXPR1` means `EXPR1` modulo `EXPR2`, and `/` is integer division).
For example:
```
(! (F (G (&&
            (== (% s 2) 0)
            (=/= (% s 7) 0)))))
```
Says "It is not the case that in the future we will globally have even integers not divisible by 7."
