unsigned int nondetUInt();

// int plusTwo(int k){
// 	return k + 2 ;
// }

int sum ()
{
    int s=0;
    
    unsigned n = nondetUInt();
    __CPROVER_assume(n>0);
    __CPROVER_assume(n<10);

//    s = plusTwo(s);
    s = s + n;
    return s;
}

int main()
{
    int a;

    a=sum();
    a = a - 10;
    assert( a>= 0);

}

