// Copyright: (C) 2009 Gilles Muller, Julia Lawall, INRIA, DIKU.  GPLv2.
@@ expression E1,E2; @@
(
  !E1 & !E2
|
- !E1 & E2
+ !(E1 & E2)
)


