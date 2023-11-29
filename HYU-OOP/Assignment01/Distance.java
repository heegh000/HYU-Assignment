public class Distance {
	private String name;
	private double lat;
	private double lng;
	
	public Distance(String name, double lat, double lng) {
		this.name = name;
		this.lat = lat;
		this.lng = lng;
	}
	
	public String writeDistance() {
		String reStr;
		
		reStr = "Country: " + this.name + 
				"\nlatitude=" + Double.toString(this.lat) + 
				"\nlongitude=" + Double.toString(this.lng) + 
				"\n--------------------";
		
		return reStr;
	}
	
	public static String getDistance(Distance a, Distance b) {
		
		double distance = Math.sqrt(Math.pow(a.lng - b.lng, 2) + Math.pow(a.lat - b.lat, 2)); 
		
		String reStr = a.writeDistance() + "\n" + b.writeDistance() + "\n" +"is\n" + Double.toString(distance)+ "\n";
		
		return reStr;
	}
}
