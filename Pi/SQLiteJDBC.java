import java.sql.*;

public class SQLiteJDBC {
  public static void main( String args[] ) {
      Connection c = null;
      Statement stmt = null;
      
      try {
         Class.forName("org.sqlite.JDBC");
         c = DriverManager.getConnection("jdbc:sqlite:FarmHealth.db");
		 
		 c.setAutoCommit(false);
      System.out.println("Opened database successfully");

      stmt = c.createStatement();
      ResultSet rs = stmt.executeQuery( "SELECT * FROM MySoilHealthTS;" );
      while ( rs.next() ) {
         int RecordID = rs.getInt("RecordID");
         String  sensorID = rs.getString("SensorID");
         String Timestamp  = rs.getString("Timestamp");
         float Temperature = rs.getFloat("Temperature");
         float Humidity = rs.getFloat("Humidity");
         float Light = rs.getFloat("Light");
         float Fertility = rs.getFloat("Fertility");
         
         System.out.println( "RecordID = " + RecordID );
         System.out.println( "sensorID = " + sensorID );
         System.out.println( "Timestamp = " + Timestamp );
         System.out.println( "Temperature = " + Temperature );
         System.out.println( "Humidity = " + Humidity );
         System.out.println( "Light = " + Light );
         System.out.println( "Fertility = " + Fertility );
         
         System.out.println();
      }
      rs.close();
      stmt.close();
      c.close();
		 
      } catch ( Exception e ) {
         System.err.println( e.getClass().getName() + ": " + e.getMessage() );
         System.exit(0);
      }
      System.out.println("Operation done successfully");
   }
}