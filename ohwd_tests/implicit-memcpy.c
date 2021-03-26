typedef unsigned char uint8_t;

struct S {
    uint8_t b[N];
};

extern struct S a, b;

void f (int i)
{
    if(i)
        a = b;
}
