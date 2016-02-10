# Relocatable-Assembler

Introduction
============

This project is a relocatable assembler implemented for MIPS Assembly Language.

* This assembler translates the MIPS Assembly language program into the machine language program (binary).
* The output program is using [MERL file format](https://www.student.cs.uwaterloo.ca/~cs241/merl/merl.html).
* This assembler only support limited MIPS instructions, see the list of [supported MIPS instructions](#Supported MIPS Instructions).


Compile
-------

Compile the program using the command:

```
g++ -o relasm relasm.cc encoder.cc lexer.cc kind.cc
```

Run
---

If reading from console (change "output" to the wanted file name):

```
./relasm > output.merl
```

If reading from a file (change "input" to the MIPS file name, "output" to the wanted file name):
```
./relasm < input > output.merl
```

Supported MIPS Instructions
---------------------------

* `.word` -- `.word i`
* `add` -- `add $d, $s, $t`
* `sub` -- `sub $d, $s, $t`
* `mult` -- `mult $s, $t`
* `multu` -- `multu $s, $t`
* `div` -- `div $s, $t`
* `divu` -- `divu $s, $t`
* `mfhi` -- `mfhi $d`
* `mflo` -- `mflo $d`
* `lis` -- `lis $d`
* `lw` -- `lw $t, i($s)`
* `sw` -- `sw $t, i($s)`
* `slt` -- `slt $d, $s, $t`
* `sltu` -- `sltu $d, $s, $t`
* `beq` -- `beq $s, $t, i`
* `bne` -- `bne $s, $t, i`
* `jr` -- `jr $s`
* `jalr` -- `jalr $s`

More details please see the [MIPS Reference Sheet](https://www.student.cs.uwaterloo.ca/~cs241/mips/mipsref.pdf).
``

Policy
------

This project must be used for learning and communicating ONLY.

Please do not use this project as part of individual assignment.

This Project is under University of Waterloo Acacemic Integrity Polity, see the details [HERE](https://www.student.cs.uwaterloo.ca/~cs241/#cheating).
