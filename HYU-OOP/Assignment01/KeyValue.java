import java.util.*;

public class KeyValue {

	private String key;
	private String value;
	
	public KeyValue(String keyEqualValue) {
		StringTokenizer st = new StringTokenizer(keyEqualValue, "=");
		this.key = "{" + st.nextToken() + "}";
		this.value = st.nextToken();
	}
	
	public KeyValue(String key, String value) {
		this.key = key;
		this.value = value;
	}
	
	public String getKey() {
		return this.key;
	}
	
	public String getValue() {
		return this.value;
	}
	
}
