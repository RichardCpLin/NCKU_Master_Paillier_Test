#include <math.h>
#include <stdio.h>
#include <time.h>
#include <stdlib.h>

long pow_mod(long long a, long b, long p) {//base,pow,mod
  long ans = 1;
  long tmp = a % p;
  while (b) {
    if (b & 1)
      ans = ans * tmp % p;
    b >>= 1;
    tmp = tmp * tmp % p;
  }
  return ans % p;
}

long g_Function(int n, int z, int t) {
  long n2 = n * n;
  long g = ((1 + n)*(z^n)) % n2;
  return g;
}

long L_Function(long long base, long LambdaN, long n) {
  long u = pow_mod(base, LambdaN, n * n);
  return (u - 1) / n;
}

long gcd(long a, long b) {
  if (b != 0) {
    return gcd(b, a % b);
  } else {
    return a;
  }
}
long findranda(long p, long q)
{
    long greater = (p > q) ? p : q;
    long randa;

    while(1) {
        if((greater % p == 0) && (greater % q == 0)) {
            randa = greater;
            break;
        }
        greater += 1;
    }
    return randa;
}

long find_rd(long randa, long n)
{
    long rd = 0;
    long value;

    while (1) 
    {
        rd += 1;
        value = (n * rd - 1) % randa;
        if (value == 0)
            break;
    }
    return rd;
}

long find_r(long c,long m,long g,long rd,long randa,long n)
{
    m = 0;
    long  neg_m = (((-m) % randa) + randa) % randa;
    long c_small = c % n;
    long g_small = g % n;
    long test_v = pow_mod(g_small, neg_m, n);// % n;
    long rvalue = pow_mod((c_small * test_v), rd, n);// % n;

    return rvalue;
}
long inv(long e, long modValue) {
	long r, r1 = modValue, r2 = e;
	long t, t1 = 0, t2 = 1;
	long q = 0;
	while (r2 > 0) {
		q = r1 / r2;
		r = r1 - q * r2;
		r1 = r2;
		r2 = r;
		t = t1 - q * t2;
		t1 = t2;
		t2 = t;
	}
	if (r1 == 1) {
		if (t1 <= 0) {
			return t1 + modValue;
		} else {
			return t1;
		}
	} else {
		printf("ERR\n");
		e = modValue - 1;
		while (gcd(e, modValue) != 1 || 1 > e || e > modValue) {
			e -= 1;
		}
		printf("E:%lu\n",e);
		//    Serial.println(e);
		inv(e, modValue);
	}
}
long encrypt(long msg, long g, long r, long n) {
  long n2 = n * n;
  long c = (pow_mod(g, msg, n2) * pow_mod(r, n, n2)) % n2;
  return c;
}
long decrypt(long long msg, long LambdaN, long u, long n) {
  long l = L_Function(msg, LambdaN, n);
  long m = (l * u) % n;
  return m;
}
int isCoprime(long a, long b)
{
	long i, hcf;
	long num1, num2;

	if(a > b)
	{
		num1 = a;
		num2 = b;
	}
	else {
		num1 = b;
		num2 = a;
	}
	// Finding HCF
	for(i=1;i<=num1;i++)
	{
		if(num1%i==0 && num2%i==0)
		{
			hcf = i;
		}
	}
	if (hcf == 1) {
		return 1;
	}
	return 0;
}
long start_find_r(long n,long base)
{
  long randv;
  long r;

  r = 0;
  while(r == 0)
  {
	  srand(time(NULL));
	  randv = base + rand() % 2000;
	  if (isCoprime(randv, n) == 1) {
		  r = randv;
	   }
  }
  return r;
}
int isPrime(long n) {
	int i;
	for (i = 2; i < n; i++) {
		if (n % i == 0) { 
			return 0;
		}
	}
	return 1;
}
long start_find_Prime(long start) {
	long randv;
	long P;

	P = 0;

	while(P == 0) {
		srand(time(NULL));
		randv = start + rand() % 30;
		if (isPrime(randv) == 1) {
			P = randv;
		}
	}
	return P;
}
int main(int argc, char *argv[])
{
  // put your setup code here, to run once:
  int i = 0;
  long p = 199;
  long q = 223;
  long t = 1;
  long n = p * q;
  long z = n - 1;
  long m1 = 50;
  long m2 = 20;

  printf("##### Define Private Key P = %lu, Q= %lu\n",p, q);
  for (i = 0; i< 5;i++) {
      printf("測試次數 %d\n",(i+1));
	  printf("=========== 傳送端 ===============\n");
	  long g = g_Function(n, z, t);
	  printf("#### Public Key (n = %lu, g =%lu)\n",n, g);
	  long r1 = start_find_r(n,7000);
      sleep(1);
	  long r2 = start_find_r(n, 7000);
	  m1 += 5;
	  m2 += 10;
	  long c1 = encrypt(m1, g, r1, n);
	  long c2 = encrypt(m2, g, r2, n);
	  printf("#### M1 = %lu, r1 = %lu, C1 = %lu\n", m1, r1, c1);
	  printf("#### M2 = %lu, r2 = %lu, C2 = %lu\n", m2, r2, c2);
	  long LambdaN = (p - 1) * (q - 1) * 0.5; //(r/2)
	  long k = L_Function(g, LambdaN, n);
	  long u = inv(k, n);
	  long TotalreMessage = decrypt(c1*c2, LambdaN, u, n);
	  long reMessageC1 = decrypt(c1, LambdaN, u, n);
	  long reMessageC2 = decrypt(c2, LambdaN, u, n);
	  printf("\n=========== 接收端 ===============\n");
	  printf("##### Paillier Decrypt 驗證\n");
	  printf("C1 Message %lu, 解密後：%lu (原:%lu)\n", c1, reMessageC1, m1);
	  printf("C2 Message %lu, 解密後：%lu (原:%lu)\n", c2, reMessageC2, m2);
	  printf("##### Paillier 同態性驗證\n");
	  printf("Total (m1 + m2): %lu\nFunDecrypt(c1*c2, key) = %lu\n", (m1 + m2),TotalreMessage);
	  printf("##### 亂數r 驗證\n");
	  long randaN = findranda(p-1,q-1);
	  long d = find_rd(randaN, n);
	  printf("λ(n) = %lu, d= %lu\n", randaN, d);
	  long rr1   = find_r(c1, TotalreMessage,g,d,randaN, n);
	  //  long rr1_1 = find_r(c1, reMessageC1,g,d,randaN, n);
	  long rr2   = find_r(c2, TotalreMessage,g,d,randaN, n);
	  //  long rr2_1 = find_r(c2, reMessageC2,g,d,randaN, n);
	  printf("r1: %lu(%lu), r2: %lu(%lu)\n\n", rr1, r1, rr2,r2);
  }
  return 0;
}

