package lyq.com.live;

import android.util.Log;

import com.google.gson.Gson;

import java.io.IOException;

import io.reactivex.Flowable;
import lyq.com.live.list.LiveList;
import lyq.com.live.room.Room;
import okhttp3.Interceptor;
import okhttp3.OkHttpClient;
import okhttp3.Response;
import retrofit2.Retrofit;
import retrofit2.adapter.rxjava2.RxJava2CallAdapterFactory;
import retrofit2.converter.gson.GsonConverterFactory;

/**
 * @author liyuqing
 * @date 2018/9/8.
 * @description 写自己的代码，让别人说去吧
 * <p>
 * 单例模式 确保一个类只有一个对象
 *
 * 1.构造方法私有化
 * 2.提供给外部一个静态接口，返回对象
 * 懒汉式：double checking ，在使用时再创建
 *
 */
public class LiveManager {


    private static LiveManager instance;
    private LiveService liveService;


    private LiveManager() {
        //Retrofit网络请求
        OkHttpClient callFactory=new OkHttpClient.Builder().addInterceptor(new Interceptor() {
            @Override
            public Response intercept(Chain chain) throws IOException {

                Response response=chain.proceed(chain.request());
                LiveList liveList=new Gson().fromJson(response.body().toString(),LiveList.class);
                System.out.println(liveList.getData());
                return response;
            }
        }).build();

        Retrofit retrofit=new Retrofit.Builder().baseUrl("http://api.m.panda.tv/")
                .addCallAdapterFactory(RxJava2CallAdapterFactory.create())
                .addConverterFactory(GsonConverterFactory.create())
                .build();

        liveService=retrofit.create(LiveService.class);

    }


    //静态方法，随类加载的时候，就被调用并且创建了该对象，
    public static LiveManager getInstance() {
        if (null == instance) { //第一次判断为空，因为单例模式只会创建一个对象，同步代码卡的消耗较大，所以判断是否为空，这样大部分情况下都不需要走到同步代码块了
            synchronized (LiveManager.class) {//可能多个线程并发调用这个方法，这样就可能会创建多个对象，所以加锁解决
                if (null == instance){//第二次判空，是为了防止比如说A，B线程都读到了第一次判空，如果不进行第二次判空则会出现多个对象
                    instance = new LiveManager();
                }
            }

        }

        return instance;
    }

    public Flowable<LiveList> getLiveList(String cate){

        return liveService.getLiveList(cate,1, 10, "3.3.1.5978");
    }

    public Flowable<Room> getLiveRoom(String id) {
        return liveService.getLiveRoom(id, "3.3.1.5978", 1, "json", "android");
    }
}
