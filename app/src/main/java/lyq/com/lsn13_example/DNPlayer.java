package lyq.com.lsn13_example;

import android.util.Log;
import android.view.Surface;
import android.view.SurfaceHolder;
import android.view.SurfaceView;

/**
 * @author liyuqing
 * @date 2018/9/8.
 * @description 写自己的代码，让别人说去吧
 * 提供Java进行播放，停止，暂停等函数
 *
 * 在准备视频的时候，准备好了才播放，如果准备失败也得有回调，
 * 所以这里准备个接口
 */
public class DNPlayer implements SurfaceHolder.Callback {

    static {
        System.loadLibrary("native-lib");
    }

    private String dataSource;
    private SurfaceHolder surfaceHolder;
    private OnPrepareListener listener;

    /**
     * 让使用者设置的播放地址
     * @param dataSource
     */
    public void setDataSource(String dataSource) {
        this.dataSource = dataSource;
    }


    public void setSurfaceView(SurfaceView surfaceView){
        if (null != surfaceHolder){
            surfaceHolder.removeCallback(this);
        }
        surfaceHolder = surfaceView.getHolder();
        surfaceHolder.addCallback(this);
    }


    /**
     * 准备好要播放的视频
     */
    public void prepare(){
        native_prepare(dataSource);
    }

    /**
     * 开始播放
     */
    public void start(){
        native_start();
    }

    /**
     * 停止播放
     */
    public void stop(){
        native_stop();

    }


    public void onPrepare(){
        if (null != listener){
            listener.onPrepare();
        }
    }

    public void onError(int errorCode){
        Log.e("自定义标签", "onError: java接收到的错误代码："+errorCode );
    }


    public void setListener(OnPrepareListener listener) {
        this.listener = listener;
    }

    /**
     * 画布创建好
     * @param surfaceHolder
     */
    @Override
    public void surfaceCreated(SurfaceHolder surfaceHolder) {

    }

    /**
     * 画布切换的时候，比如横竖屏，按home键
     * @param surfaceHolder
     * @param i
     * @param i1
     * @param i2
     */
    @Override
    public void surfaceChanged(SurfaceHolder surfaceHolder, int i, int i1, int i2) {
        natice_setSurface(surfaceHolder.getSurface());
    }

    /**
     * 画布被销毁
     * @param surfaceHolder
     */
    @Override
    public void surfaceDestroyed(SurfaceHolder surfaceHolder) {

    }

    /**
     * 当不播放视频的时候，要把接口释放
     */
    public void release(){
        surfaceHolder.removeCallback(this);
        native_release();
    }

    public interface OnPrepareListener{
        void onPrepare();
    }

    native void native_prepare(String dataSource);
    native void native_start();
    native void natice_setSurface(Surface surface);
    native void native_stop();
    native void native_release();
}
