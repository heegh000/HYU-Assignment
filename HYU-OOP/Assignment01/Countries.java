import java.util.Scanner;
import java.util.StringTokenizer;
import java.io.FileInputStream;
import java.io.FileNotFoundException;

public class Countries {
	private static String[][] countryArr;
	private String country;
	private double lat;
	private double lng;
	
	static {
	
		try {
		Scanner fileIn = new Scanner(new FileInputStream("Countries.csv"));
		countryArr = new String[245][3];
		
		int i = 0;
		String curLine = null;
		StringTokenizer st = null;
		
		while(fileIn.hasNext()) {
			curLine = fileIn.nextLine();
			
			st = new StringTokenizer(curLine, ",");
			
			countryArr[i][0] = st.nextToken();
			countryArr[i][1] = st.nextToken();
			countryArr[i][2] = st.nextToken();
			
			i++;
		}
		
		fileIn.close();
		
		
		} 
		
		catch(FileNotFoundException e) {
			System.exit(0);
		}
		
		
	}
	
	public Countries(String country) {
		for(int i = 0; i < 245; i++) {
			if(countryArr[i][0].equals(country)) {
				this.country = countryArr[i][0];
				this.lat = Double.valueOf(countryArr[i][1]);
				this.lng = Double.valueOf(countryArr[i][2]);
				break;
			}
		}
		
	}
	
	public String getCountry() {
		return this.country;
	}
	
	public double getLat() {
		return this.lat;
	}
	
	public double getLng() {
		return this.lng;
	}
	
}
