package lyq.com.live.list;

/**
 * @author liyuqing
 * @date 2018/9/8.
 * @description 写自己的代码，让别人说去吧
 */
public class LiveList {

    private int errno;
    private String errmsg;
    private Data data;

    public int getErrno() {
        return errno;
    }

    public void setErrno(int errno) {
        this.errno = errno;
    }

    public String getErrmsg() {
        return errmsg;
    }

    public void setErrmsg(String errmsg) {
        this.errmsg = errmsg;
    }

    public Data getData() {
        return data;
    }

    public void setData(Data data) {
        this.data = data;
    }
}
