// Nghiem, Hunter
// hxn5883
// 2018-12-3

import java.util.function.UnaryOperator;
import java.util.function.BinaryOperator;

public class hmwk_04_functional {
  // Sum of squares lambda
  static UnaryOperator <Long>
    sumOfSquares = (Long n) -> n == 0 ? 0 :(n*n + hmwk_04_functional.sumOfSquares.apply(n-1));

  // Pell numbers lambda
  static UnaryOperator <Long>
    pell = (Long n) -> n ==0 ? 0 : ((n == 1) ? 1 : (2*hmwk_04_functional.pell.apply(n-1) + hmwk_04_functional.pell.apply(n-2)));

  // Powers lambda
    static BinaryOperator <Long>
    powers = (Long m, Long n) -> (n==0) ? (m-1) : (m-1 + m * hmwk_04_functional.powers.apply(m,n-1));

  //----------------------------------------------------------
  public static void main( String[] args )
  {
    for ( Long i = 0L; i <= 15; i++ ) {
      System.out.format( "sumOfSquares(%d) is %d\n", i, sumOfSquares.apply(i) );
    }

    for ( Long i = 0L; i <= 15; i++ ) {
      System.out.format( "pell(%d) is %d\n", i, pell.apply(i) );
    }

    for ( Long i = 2L; i <= 10; i++ ) {
      for ( Long j = 1L; j <= 10; j++ ) {
        System.out.format( "powers(%d, %d) is %d\n", i, j, powers.apply( i, j ) );
      }
    }
  }
}
