package lyq.com.live;

import io.reactivex.Flowable;
import lyq.com.live.list.LiveList;
import lyq.com.live.room.Room;
import retrofit2.http.GET;
import retrofit2.http.Query;

/**
 * @author liyuqing
 * @date 2018/9/8.
 * @description 写自己的代码，让别人说去吧
 *
 * Retrofit+Rxjava+MVP结构
 *
 * http://api.m.panda.tv/ajax_get_live_list_by_cate?cate=lol&pageno=1&pagenum=1&room=1&version=3.3.1.5978
 *
 * http://api.m.panda.tv/ajax_get_liveroom_baseinfo?roomid=237908&__version=3.3.1.5978&slaveflag=1&type=json&__plat=android
 *  roomid  第一个请求地址获得的json中的id
 */
public interface LiveService {

    @GET("ajax_get_live_list_by_cate")
    Flowable<LiveList> getLiveList(@Query("cate") String cate, @Query("pageno") int pageno, @Query
            ("pagenum") int pagenum, @Query("version") String version);

    @GET("ajax_get_liveroom_baseinfo")
    Flowable<Room> getLiveRoom(@Query("roomid") String roomid, @Query("__version") String
            __version, @Query("slaveflag") int slaveflag, @Query("type") String type, @Query
                                       ("__plat") String __plat);


}
