class Main inherits IO {
  a: Int <- blah();
  blah(): Int { a + 1 };
  main(): Object { out_int(a) };
};