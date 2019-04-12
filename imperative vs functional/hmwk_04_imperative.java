// Nghiem, Hunter
// hxn5883
// 2018-12-3

public class hmwk_04_imperative {
  // Sum of squares
  static long sumOfSquares(Long n)
  {
    if(n==0)
    {
      return 0;
    }
    else
    {
      return n*n + sumOfSquares(n-1);
    }
  }
  // Pell numbers
  static long pell(Long n)
  {
    if(n==0)
    {
      return 0;
    }
    else if (n ==1)
    {
      return 1;
    }
    else
    {
      return 2 * pell(n-1) + pell(n-2);
    }
  }

  // Powers
  static long powers(Long m, Long n)
  {
    if(n==0)
    {
      return m-1;
    }
    else
    {
      return m-1 + m * powers(m, n-1);
    }

  }
  //----------------------------------------------------------
  public static void main( String[] args )
  {
    for ( Long i = 0L; i <= 15; i++ ) {
      System.out.format( "sumOfSquares(%d) is %d\n", i, sumOfSquares(i) );
    }

    for ( Long i = 0L; i <= 15; i++ ) {
      System.out.format( "pell(%d) is %d\n", i, pell(i) );
    }

    for ( Long i = 2L; i <= 10; i++ ) {
      for ( Long j = 1L; j <= 10; j++ ) {
        System.out.format( "powers(%d, %d) is %d\n", i, j, powers( i, j ) );
      }
    }
  }
}
