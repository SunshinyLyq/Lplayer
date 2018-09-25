package lyq.com.lsn13_example;

import android.content.Intent;
import android.media.AudioRecord;
import android.support.design.widget.TabLayout;
import android.os.Bundle;
import android.support.v7.widget.LinearLayoutManager;
import android.support.v7.widget.RecyclerView;
import android.util.Log;

import com.trello.rxlifecycle2.android.ActivityEvent;
import com.trello.rxlifecycle2.components.support.RxAppCompatActivity;

import io.reactivex.android.schedulers.AndroidSchedulers;
import io.reactivex.schedulers.Schedulers;
import io.reactivex.subscribers.DisposableSubscriber;
import lyq.com.live.LiveManager;
import lyq.com.live.list.LiveList;
import lyq.com.live.room.Room;
import lyq.com.live.room.Videoinfo;


/**
 * 需求：
 * 1.Tablayout形式来展示直播，播放直播
 *
 * 分析：
 * 1.怎么获取直播地址，数据格式是什么样的
 * 2.怎么请求网络框架
 * 写法：单独写成一个module library
 *
 *
 */

public class MainActivity extends  RxAppCompatActivity implements LiveAdapter.OnItemClickListener, TabLayout.BaseOnTabSelectedListener {


    private RecyclerView recyclerView;
    private LiveAdapter liveAdapter;
    private TabLayout tabLayout;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        //配置recycleView
        recyclerView= findViewById(R.id.recyclerView);
        LinearLayoutManager linearLayoutManager=new LinearLayoutManager(this);
        linearLayoutManager.setOrientation(LinearLayoutManager.VERTICAL);
        recyclerView.setLayoutManager(linearLayoutManager);

        //设置适配器
        liveAdapter=new LiveAdapter(this);
        liveAdapter.setmOnItemClickListener(this);
        recyclerView.setAdapter(liveAdapter);
        
        //配置tablayout
        tabLayout =findViewById(R.id.tabLayout);
        tabLayout.addOnTabSelectedListener(this);

        addTabs();


    }

    private void addTabs() {
        addTab("lol","英雄联盟");
        addTab("acg","二次元");
        addTab("food","美食");
    }

    private void addTab(String tag, String name) {

        TabLayout.Tab tab=tabLayout.newTab();
        tab.setTag(tag);
        tab.setText(name);
        tabLayout.addTab(tab);
    }


    @Override
    public void onItemClick(String id) {
        //获取roomid，然后点击跳转到playActivity
        LiveManager.getInstance()
                .getLiveRoom(id)
                .compose(this.<Room>bindUntilEvent(ActivityEvent.DESTROY))
                .subscribeOn(Schedulers.io())
                .observeOn(AndroidSchedulers.mainThread())
                .subscribe(new DisposableSubscriber<Room>() {


                    @Override
                    public void onNext(Room room) {
                        Videoinfo info = room.getData().getInfo().getVideoinfo();
                        String room_key = info.getRoom_key();
                        String sign = info.getSign();
                        String ts = info.getTs();
                        Intent intent = new Intent(MainActivity.this, PlayActivity.class);
                        intent.putExtra("url", "http://pl3.live.panda.tv/live_panda/" + room_key
                                + "_mid" +
                                ".flv?sign=" + sign +
                                "&time=" + ts);
                        startActivity(intent);
                    }

                    @Override
                    public void onError(Throwable t) {
                        t.printStackTrace();
                    }

                    @Override
                    public void onComplete() {

                    }
                });

    }

    @Override
    public void onTabSelected(TabLayout.Tab tab) {
        //请求获取房间
        // TODO: 2018/9/8 显示加载等待
        LiveManager.getInstance().getLiveList(tab.getTag().toString())
                .compose(this.<LiveList>bindUntilEvent(ActivityEvent.DESTROY))
                .subscribeOn(Schedulers.io())
                .observeOn(AndroidSchedulers.mainThread())
                .subscribe(new DisposableSubscriber<LiveList>() {
                    @Override
                    public void onNext(LiveList liveList) {

                        liveAdapter.setItems(liveList);
                        liveAdapter.notifyDataSetChanged();
                    }

                    @Override
                    public void onError(Throwable t) {

                    }

                    @Override
                    public void onComplete() {
                    }
                });
    }

    @Override
    public void onTabUnselected(TabLayout.Tab tab) {

    }

    @Override
    public void onTabReselected(TabLayout.Tab tab) {

    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        tabLayout.removeOnTabSelectedListener(this);
    }
}
