class A inherits IO {
    bleh() : Object { out_string("bloop") };
};

class B inherits A {
    bleh() : Object { 9999 };
};

class Main inherits IO {
    addOne(val : Int) : Int { 6 };

    a : Int <- 1234;
  main() : Int {
    {
        a <- 8549;
        123;
    }
  } ;
} ; 
