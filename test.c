#include <stdlib.h>
#include <stdio.h>
  
  typedef struct{
    double amount;
    const char* currency;
  }t_money;

t_money test() {
  t_money money;
  money.amount = 15.0;
  money.currency = "USD";
  return money;
}

int main() {
  t_money b = test();
  printf("%f %s", b.amount, b.currency);
}