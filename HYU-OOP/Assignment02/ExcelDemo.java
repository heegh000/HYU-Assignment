import java.awt.Color;
import java.awt.Component;
import java.awt.Dimension;
import java.awt.Font;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.awt.event.MouseAdapter;
import java.awt.event.MouseEvent;
import java.io.BufferedReader;
import java.io.BufferedWriter;
import java.io.FileReader;
import java.io.FileWriter;
import java.util.StringTokenizer;

import javax.swing.JComponent;
import javax.swing.JFileChooser;
import javax.swing.JFrame;
import javax.swing.JMenu;
import javax.swing.JMenuBar;
import javax.swing.JMenuItem;
import javax.swing.JOptionPane;
import javax.swing.JScrollPane;
import javax.swing.JTable;
import javax.swing.SwingConstants;
import javax.swing.filechooser.FileNameExtensionFilter;
import javax.swing.table.DefaultTableCellRenderer;
import javax.swing.table.DefaultTableModel;

@SuppressWarnings("serial")
public class ExcelDemo extends JFrame{
	
	JScrollPane scrollPane;
	JTable table, headerTable;
	JMenuBar menuBar;
	JMenu fileMenu, formulaMenu, functionMenu;
	JMenuItem newItem, open, save, exit, sum, average, count, max, min;
	String title;
	int cardinality, degree;
	
	public ExcelDemo() {
		
		setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE); //X버튼 누를시 종료
		setTitle("새 Microsoft Excel 워크시트.xlsx - Excel"); //프레임 제목 설정
		
		table = new JTable(100, 26); //table 생성
		
		table.getTableHeader().setReorderingAllowed(false); //table의 column들 위치 변경 방지
		table.setAutoResizeMode(JTable.AUTO_RESIZE_OFF); //table의 cell들 크기 변경 방지
		table.addMouseListener(new ExcelMouseAdapter()); //table에 mouse 이벤트 추가
		
		
		//headerTable을 만들기 위한 배열 할당
		String hTable[] = {" "}; 
		String hTableData[][] = new String[100][1];
		for(int i = 0; i < 100; i++)  {
			hTableData[i][0] = Integer.toString(i);
		}
		
		//headerTable 수정 금지를 위한 DefaultTableModel을 익명 class로 선언하여 isCellEditable 오버라이딩
		DefaultTableModel model = new DefaultTableModel(hTableData, hTable) { 
		    public boolean isCellEditable(int row, int col) { 
		     return false; 
		    } 
		}; 
		
		headerTable = new JTable(model); //headerTable 생성
		
		headerTable.setAutoResizeMode(JTable.AUTO_RESIZE_OFF); //headerTable의 cell들 크기 변경 방지
		headerTable.setFocusable(false); //headerTable의 cell 더블 클릭 제한
		headerTable.getColumnModel().getColumn(0).setPreferredWidth(50); //headerTable의 cell 너비 설정
		headerTable.setPreferredScrollableViewportSize(new Dimension(50,0)); //headerTable 보이는 범위 설정
		headerTable.setBackground(new Color(238, 238, 238)); //headerTable의 cell 색 변경
		headerTable.setSelectionModel(table.getSelectionModel()); //table의 cell이 선택되었을때 같은 row에 있는 headerTable의 cell도 선택되게 바꿈
		
		//cell이 선택 되었을때 글자가 진하게 표시되기 위해 DefaultTableCellRenderer을 익명 class로 선언하여 getTableCellRendererComponent 오버라이딩
		DefaultTableCellRenderer cellRenderer = new DefaultTableCellRenderer() { 
			public Component getTableCellRendererComponent(JTable table, Object value, boolean isSelected, boolean hasFocus, int row, int column) {
				JComponent c = (JComponent) super.getTableCellRendererComponent(table, value, isSelected, hasFocus, row, column);
			
				if(isSelected)
					c.setFont(getFont().deriveFont(Font.BOLD));
				
				return c;
			}
		};
		
		cellRenderer.setHorizontalAlignment(SwingConstants.CENTER); //글자 가운데 정렬
		headerTable.setDefaultRenderer(headerTable.getColumnClass(0), cellRenderer); //headerTable에 cellRenderer을 적용
		
		scrollPane = new JScrollPane(table); //테이블을 기준으로 scorllPane 생성 
		scrollPane.setVerticalScrollBarPolicy(JScrollPane.VERTICAL_SCROLLBAR_AS_NEEDED); //수직 스크롤바가 필요할때 보이게 설정
		scrollPane.setHorizontalScrollBarPolicy(JScrollPane.HORIZONTAL_SCROLLBAR_AS_NEEDED); //수평 스크롤바가 필요할때 보이게 설정
		scrollPane.setRowHeaderView(headerTable); //headerTable을 row Header로 설정
				
		add(scrollPane); //프레임에 scorllPane을 추가
		
		createMenu(); //메뉴 붙임
		
		setSize(610, 588); //프레임 크기 지정
		setLocationRelativeTo(null); //프레임을 화면 중앙에 띄움
		setVisible(true); //프레임 보이게 설정
	}

	
	public void createMenu() {
		
		menuBar = new JMenuBar(); //메뉴바 생성
		
		//메뉴 생성
		fileMenu = new JMenu("File");
		formulaMenu = new JMenu("Formula");
		functionMenu = new JMenu("Function");
		
		//메뉴아이템 생성
		newItem = new JMenuItem("New");
		open = new JMenuItem("Open");
		save = new JMenuItem("Save");
		exit = new JMenuItem("Exit");
		sum = new JMenuItem("Sum");
		average = new JMenuItem("Average");
		count = new JMenuItem("Count");
		max = new JMenuItem("Max");
		min = new JMenuItem("Min");
		
		//fileMenu 에 new, open, save, exit 붙임
		fileMenu.add(newItem);
		fileMenu.add(open);
		fileMenu.add(save);
		fileMenu.add(exit);
		
		//functionMenu에 sum, average, count, max, min 붙임
		functionMenu.add(sum);
		functionMenu.add(average);
		functionMenu.add(count);
		functionMenu.add(max);
		functionMenu.add(min);
		
		formulaMenu.add(functionMenu); //formulaMenu에 functionMuenu를 붙임
		
		//menuBar에 file, formula 붙이기
		menuBar.add(fileMenu);
		menuBar.add(formulaMenu);
		
		ExcelActionListener excelEvent = new ExcelActionListener(); //메뉴 이벤트 처리를 위한  ExcelActionListener 객체 생성
		
		//각각의 Item에 이벤트를 적용
		newItem.addActionListener(excelEvent);
		open.addActionListener(excelEvent);
		save.addActionListener(excelEvent);
		exit.addActionListener(excelEvent);
		sum.addActionListener(excelEvent);
		average.addActionListener(excelEvent);
		count.addActionListener(excelEvent);
		max.addActionListener(excelEvent);
		min.addActionListener(excelEvent);
		
		//프레임에 메뉴바 붙이기
		setJMenuBar(menuBar);
	}	
	
	//메뉴에 따른 이벤트를 위한 내부 class
	private class ExcelActionListener implements ActionListener {
		public void actionPerformed(ActionEvent e) {
			
			//New가 눌렸을떄는 현재 프레임을 종료하고 새로운 프레임을 실행시킴
			if(e.getSource() == newItem) {
				dispose(); //하나의 프레임을 종료
				new ExcelDemo(); //새로운 프레임 호출
			}
			
			
			
			//Open이 눌렸을때는 파일을 읽어와서 현재 프레임을 종료하고 새로운 프레임을 실행시켜 데이터를 입력함
			else if (e.getSource() == open) {
				JFileChooser fc = new JFileChooser();

				//탐색하고싶은 확장자 필터링
				fc.addChoosableFileFilter(new FileNameExtensionFilter("txt 파일", "txt"));
				fc.addChoosableFileFilter(new FileNameExtensionFilter("csv 파일", "csv"));

				int select = fc.showOpenDialog(null); //OpenDiaglog를 실행
				
				//파일을 열지 않았을때 처리
				if(select != JFileChooser.APPROVE_OPTION) 
	                return;
				
				//프레임 제목 바꾸기
				title = fc.getSelectedFile().getPath();
				setTitle(title);
				
				//기존 데이터 지우기
				for(int row = 0; row < 100; row++) {
					for(int col = 0; col < 26; col++) {
						table.setValueAt("", row, col);
					}
				}
				
				int row = 0;
				int col = 0;
				
				try {
					//파일에서 값을 읽어오기 위한 FileReader, BufferedReader
					FileReader fr = new FileReader(fc.getSelectedFile());
					BufferedReader br = new BufferedReader(fr);
					
					String line = "";
					String cell = null;
					
					//입력 파일에서 데이터를 읽어와 table에 입력하는 과정
					while(line != null) {
						line = br.readLine(); //입력 파일에서 한 라인씩 읽음

						//,,입력을 처리 ex)1,2,3,,4로 입력이 들어오면 1,2,3, ,4로 바꿔줌
						String[] split = line.split(",");
						line = "";
						for(int i = 0; i < split.length; i++) { 
							if(split[i].equals(""))
								line += " ,";
							else
								line += split[i] + ",";
						}

						StringTokenizer st = new StringTokenizer(line, ","); //,을 기준으로 tokenizing
						
						col = 0;
						
						//입력받은 한 라인을 처리
						while(st.hasMoreTokens()) {
							cell = st.nextToken();
			
							//위에서 ,,입력을 , ,로 처리했으므로 " "가 들어오면 ""을 table에 입력, 아니면 그냥 입력
							if(cell.equals(" "))
								table.setValueAt("", row, col);
							else
								table.setValueAt(cell, row, col);
							col++; //nextToken으로 넘어기전에 다음 column으로 넘어감
						}
						
						row++;//다음 라인로 넘어가기 전에 다음 row로 넘어감
					}
					
					//FileReader, BufferedReaderd을 닫음
					br.close();
					fr.close();
				}
				catch (Exception exc) {
				}
			}
			
			
			
			//Save를 눌렀을때 현재 table에 저장된 정보를 파일로 저장
			else if (e.getSource() == save) {
				JFileChooser fc = new JFileChooser();

				//탐색하고싶은 확장자 필터링
				fc.addChoosableFileFilter(new FileNameExtensionFilter("txt 파일", "txt"));
				fc.addChoosableFileFilter(new FileNameExtensionFilter("csv 파일", "csv"));

				int select = fc.showSaveDialog(null);
				
				//파일을 열지 않았을때 처리
				if(select != JFileChooser.APPROVE_OPTION) 
	                return;
				
				//프레임 제목 바꾸기
				title = fc.getSelectedFile().getPath();
				setTitle(title);
				
				String toWriteLine = "";
				
				try {
					//파일에 값을 저장하기 위한 FileWriter, BufferedWriter 
					FileWriter fw = new FileWriter(fc.getSelectedFile());
					BufferedWriter bw = new BufferedWriter(fw);
					
					//한 라인씩 파일에 입력
					for(int row = 0; row < 100; row++) {
						
						//table의 cell에 null값이 저장되있으면 그냥 , 아니면 (값),를 toWriteLine에 저장
						for(int col = 0; col < 26; col++) {
							if(table.getValueAt(row, col) == null) 
								toWriteLine += ",";
							else
								toWriteLine += table.getValueAt(row, col).toString() + ",";
						}
						
						bw.write(toWriteLine); //버퍼에 쓸 라인을 저장
						bw.flush(); //버퍼에 있는 라인을 파일에 저장
						bw.newLine(); //라인 개행
						toWriteLine = ""; //쓸 라인 초기화
					}
				
					//FileWriter, BufferedWriter을 닫음
					bw.close();
					fw.close();
				}
				catch (Exception exc) {
				}
			}
			
			//Exit를 눌렀을때 하나의 프레임을 종료
			else if (e.getSource() == exit) {
				dispose();
			}
			
			
			
			//Sum을 눌렀을때 범위내에 있는 숫자들을 모두 더함
			else if (e.getSource() == sum) {
				//범위를 입력받음
				String str = JOptionPane.showInputDialog("Function Arguments");
				
				//범위를 입력받지 않았을때 처리
				if(str == null)
					return;
				
				StringTokenizer st = new StringTokenizer(str, ":"); //입력받은 범위를 :기준으로 tokenizing
				String first = st.nextToken(); //첫번째 범위
				String second = st.nextToken(); //두번째 범위
				
				//아스키코드값을 빼주어 column index를 구함
				int colStart = first.charAt(0) - 65;
				int colEnd = second.charAt(0) - 65;
				
				//row index를 구함
				int rowStart = Integer.parseInt(first.substring(1));
				int rowEnd = Integer.parseInt(second.substring(1));

				double sum = 0;
				boolean test = true; //모두 숫자가 아닐때를 확인하기 위한 boolean 변수
				
				//총합 구하기
				for(int row = rowStart; row < rowEnd + 1; row++) {
					for(int col = colStart; col < colEnd + 1; col++) {
						if(table.getValueAt(row, col) != null)  
							if(isNumber(table.getValueAt(row, col).toString())) { //null이 아니고 숫자 일때만  더해줌
								sum += Double.parseDouble(table.getValueAt(row, col).toString());
								test = false;
							}
					}
				}
				
				if(test) //범위내에 숫자가 없는 경우를 처리
					table.setValueAt(0, cardinality, degree);
				else
					table.setValueAt(Double.toString(sum), cardinality, degree);
			}
			
			
			
			//Average을 눌렀을때 범위내에 있는 숫자들의 평균을 구함
			else if (e.getSource() == average) {
				//범위를 입력받음
				String str= JOptionPane.showInputDialog("Function Arguments");
				
				//범위를 입력받지 않았을때 처리
				if(str == null)
					return;
				
				StringTokenizer st = new StringTokenizer(str, ":"); //입력받은 범위를 :기준으로 tokenizing
				String first = st.nextToken(); //첫번째 범위
				String second = st.nextToken(); //두번째 범위
				
				//아스키코드값을 빼주어 column index를 구함
				int colStart = first.charAt(0) - 65;
				int colEnd = second.charAt(0) - 65;
				
				//row index를 구함
				int rowStart = Integer.parseInt(first.substring(1));
				int rowEnd = Integer.parseInt(second.substring(1));

				double sum = 0;
				int count = 0;
				boolean test = true; //모두 숫자가 아닐때를 확인하기 위한 boolean 변수
				
				//총합 구하기
				for(int row = rowStart; row < rowEnd + 1; row++) {
					for(int col = colStart; col < colEnd + 1; col++) {
						if(table.getValueAt(row, col) != null)
							if(isNumber(table.getValueAt(row, col).toString())) { //null이 아니고 숫자 일때만 더하고, count를 늘림
								sum += Double.parseDouble(table.getValueAt(row, col).toString());
								count++;
								test = false;	
							}
					}
				}
								
				if(test) //범위내에 숫자가 없는 경우를 처리
					table.setValueAt("Can't Div 0", cardinality, degree);
				else {
					double average = sum / count;
					table.setValueAt(Double.toString(average), cardinality, degree);
				}
			}
			
			
			
			//Count을 눌렀을때 범위내에 있는 숫자들의 갯수를 셈
			else if (e.getSource() == count) {
				//범위를 입력받음
				String str= JOptionPane.showInputDialog("Function Arguments");
				
				//범위를 입력받지 않았을때 처리
				if(str == null)
					return;
				
				StringTokenizer st = new StringTokenizer(str, ":"); //입력받은 범위를 :기준으로 tokenizing
				String first = st.nextToken(); //첫번째 범위
				String second = st.nextToken(); //두번째 범위
				
				//아스키코드값을 빼주어 column index를 구함
				int colStart = first.charAt(0) - 65;
				int colEnd = second.charAt(0) - 65;
				
				//row index를 구함
				int rowStart = Integer.parseInt(first.substring(1));
				int rowEnd = Integer.parseInt(second.substring(1));
				
				int count = 0;
				
				//숫자 갯수 구하기
				for(int row = rowStart; row < rowEnd + 1; row++) {
					for(int col = colStart; col < colEnd + 1; col++) {
						if(table.getValueAt(row, col) != null)
							if(isNumber(table.getValueAt(row, col).toString())) //null이 아니고 숫자일때만 count를 늘림
									count++;
					}
				}
				
				table.setValueAt(Integer.toString(count), cardinality, degree);
			}
			
			
			
			//Max를 눌렀을때 범위 내 최댓값을 구함
			else if (e.getSource() == max) {
				//범위를 입력받음
				String str= JOptionPane.showInputDialog("Function Arguments");
				
				//범위를 입력받지 않았을때 처리
				if(str == null)
					return;
				
				StringTokenizer st = new StringTokenizer(str, ":"); //입력받은 범위를 :기준으로 tokenizing
				String first = st.nextToken(); //첫번째 범위
				String second = st.nextToken(); //두번째 범위
				
				//아스키코드값을 빼주어 column index를 구함
				int colStart = first.charAt(0) - 65;
				int colEnd = second.charAt(0) - 65;
				
				//row index를 구함
				int rowStart = Integer.parseInt(first.substring(1));
				int rowEnd = Integer.parseInt(second.substring(1));
				
				Double max = Double.MIN_VALUE;
				boolean test = true; //모두 숫자가 아닐때를 확인하기 위한 boolean 변수
				
				//최대값 구하기
				for(int row = rowStart; row < rowEnd + 1; row++) {
					for(int col = colStart; col < colEnd + 1; col++) {
						if(table.getValueAt(row, col) != null)
							if(isNumber(table.getValueAt(row, col).toString())) { //null이 아니고 숫자일때만 max를 갱신함
								if(max < Double.parseDouble(table.getValueAt(row, col).toString()))
									max = Double.parseDouble(table.getValueAt(row, col).toString());
								test = false;
							}
					}
				}
				
				if(test) //범위내에 숫자가 없는 경우를 처리
					table.setValueAt("", cardinality, degree);
				else
					table.setValueAt(Double.toString(max), cardinality, degree);
			}
			
			
			
			//Max를 눌렀을때 범위 내 최솟값을 구함
			else if (e.getSource() == min) {
				String str= JOptionPane.showInputDialog("Function Arguments");
				
				//범위를 입력받지 않았을때 처리
				if(str == null)
					return;
				
				StringTokenizer st = new StringTokenizer(str, ":"); //입력받은 범위를 :기준으로 tokenizing
				String first = st.nextToken(); //첫번째 범위
				String second = st.nextToken(); //두번째 범위
				
				//아스키코드값을 빼주어 column index를 구함
				int colStart = first.charAt(0) - 65;
				int colEnd = second.charAt(0) - 65;
				
				//row index를 구함
				int rowStart = Integer.parseInt(first.substring(1));
				int rowEnd = Integer.parseInt(second.substring(1));
				
				Double min = Double.MAX_VALUE;
				boolean test = true; //모두 숫자가 아닐때를 확인하기 위한 boolean 변수
				
				//최솟값 구하기
				for(int row = rowStart; row < rowEnd + 1; row++) {
					for(int col = colStart; col < colEnd + 1; col++) {
						if(table.getValueAt(row, col) != null)
							if(isNumber(table.getValueAt(row, col).toString())) { //null이 아니고 숫자일때만 최솟값을 갱신함
								if(min > Double.parseDouble(table.getValueAt(row, col).toString()))
									min = Double.parseDouble(table.getValueAt(row, col).toString());
								test = false;
							}
					}
				}
				
				if(test) //범위내에 숫자가 없는 경우를 처리
					table.setValueAt("", cardinality, degree);
				else
					table.setValueAt(Double.toString(min), cardinality, degree);
			}
		}
	}
	
	//마우스 클릭했을때의 위치를 저장하기 위한 class
	private class ExcelMouseAdapter extends MouseAdapter {
		public void mouseClicked(MouseEvent e) {
			//선택된 row, column의 index를 cardinality, degree에 저장
			cardinality = table.getSelectedRow();
			degree = table.getSelectedColumn();
		}
	}
	
	//메뉴 이벤트 구현에 필요한 숫자인지 아닌지 확인해주는 static 함수
	public static boolean isNumber(String str) {
		boolean isNumber;
		try {
			Double.parseDouble(str);
			isNumber = true;
			return isNumber;
		}
		catch(Exception e) {
			isNumber = false;
			return isNumber;
		}
		
	}
	
	//main함수
	public static void main(String args[]) {
		ExcelDemo ed = new ExcelDemo();
	}
}

