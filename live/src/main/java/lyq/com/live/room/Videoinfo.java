/**
 * Copyright 2018 bejson.com
 */
package lyq.com.live.room;

/**
 * Auto-generated: 2018-09-07 14:33:0
 *
 * @author bejson.com (i@bejson.com)
 * @website http://www.bejson.com/java2pojo/
 */
public class Videoinfo{

    private String room_key;
    private String plflag;
    private String sign;
    private String ts;

    public void setRoom_key(String room_key) {
        this.room_key = room_key;
    }

    public String getRoom_key() {
        return room_key;
    }

    public void setPlflag(String plflag) {
        this.plflag = plflag;
    }

    public String getPlflag() {
        return plflag;
    }


    public void setSign(String sign) {
        this.sign = sign;
    }

    public String getSign() {
        return sign;
    }

    public void setTs(String ts) {
        this.ts = ts;
    }

    public String getTs() {
        return ts;
    }

    @Override
    public String toString() {
        return "Videoinfo{" +
                "room_key='" + room_key + '\'' +
                ", plflag='" + plflag + '\'' +
                ", sign='" + sign + '\'' +
                ", ts='" + ts + '\'' +
                '}';
    }
}