package javax.microedition.lcdui;
public class TextField extends Item {
 public static final int ANY=0,EMAILADDR=1,NUMERIC=2,PHONENUMBER=3,URL=4,DECIMAL=5,CONSTRAINT_MASK=65535,PASSWORD=65536,UNEDITABLE=131072,SENSITIVE=262144,NON_PREDICTIVE=524288,INITIAL_CAPS_WORD=1048576,INITIAL_CAPS_SENTENCE=2097152;
 private String text="";private int maximum,constraints,caret;
 public TextField(String label,String value,int max,int type){super(label);if(max<1)throw new IllegalArgumentException();maximum=max;setConstraints(type);setString(value);}
 public String getString(){return text;}public void setString(String value){value=value==null?"":value;validate(value);text=value;caret=text.length();}
 private void validate(String value){if(value.length()>maximum)throw new IllegalArgumentException();int kind=constraints&CONSTRAINT_MASK;if(kind==NUMERIC||kind==DECIMAL){for(int i=0;i<value.length();i++){char c=value.charAt(i);if(c>='0'&&c<='9'||c=='-'&&i==0||kind==DECIMAL&&c=='.'&&value.indexOf('.')==i)continue;throw new IllegalArgumentException();}}}
 public int size(){return text.length();}public int getMaxSize(){return maximum;}public int setMaxSize(int max){if(max<1)throw new IllegalArgumentException();maximum=max;if(text.length()>max)setString(text.substring(0,max));return max;}
 public int getCaretPosition(){return caret;}public int getConstraints(){return constraints;}public void setConstraints(int value){int type=value&CONSTRAINT_MASK;if(type>DECIMAL)throw new IllegalArgumentException();constraints=value;try{validate(text);}catch(IllegalArgumentException e){text="";caret=0;}}
 public int getChars(char[] target){if(target==null)throw new NullPointerException();if(target.length<text.length())throw new ArrayIndexOutOfBoundsException();text.getChars(0,text.length(),target,0);return text.length();}
 public void setChars(char[] data,int offset,int length){if(data==null){setString("");return;}setString(new String(data,offset,length));}
 public void insert(String value,int at){if(value==null)throw new NullPointerException();at=Math.max(0,Math.min(text.length(),at));setString(text.substring(0,at)+value+text.substring(at));caret=at+value.length();}
 public void insert(char[] value,int offset,int length,int at){insert(new String(value,offset,length),at);}public void delete(int offset,int length){if(offset<0||length<0||offset>text.length()-length)throw new StringIndexOutOfBoundsException();setString(text.substring(0,offset)+text.substring(offset+length));caret=offset;}
 public void setInitialInputMode(String mode){}
 void render(Graphics g,int x,int y,int width){super.render(g,x,y,width);String value=text;if((constraints&PASSWORD)!=0){StringBuffer b=new StringBuffer();for(int i=0;i<text.length();i++)b.append('*');value=b.toString();}g.drawString(value,x,y+16,20);}int height(){return 38;}
}
