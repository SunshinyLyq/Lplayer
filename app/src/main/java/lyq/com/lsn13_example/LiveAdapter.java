package lyq.com.lsn13_example;

import android.content.Context;
import android.support.annotation.NonNull;
import android.support.v7.widget.RecyclerView;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.ImageView;
import android.widget.TextView;


import com.squareup.picasso.Picasso;

import java.util.ArrayList;
import java.util.List;

import lyq.com.live.list.Items;
import lyq.com.live.list.LiveList;

/**
 * @author liyuqing
 * @date 2018/9/8.
 * @description 写自己的代码，让别人说去吧
 */
public class LiveAdapter extends RecyclerView.Adapter<LiveAdapter.MyHolder> implements View.OnClickListener {

    private LayoutInflater layoutInflater;
    private List<Items> items;
    private OnItemClickListener mOnItemClickListener;
    private Context context;


    public LiveAdapter(Context context){
        this.context=context;
        layoutInflater=LayoutInflater.from(context);
        items=new ArrayList<>();
    }


    @NonNull
    @Override
    public MyHolder onCreateViewHolder(@NonNull ViewGroup viewGroup, int i) {

        View view=layoutInflater.inflate(R.layout.room_item,viewGroup,false);

        view.setOnClickListener(this);
        return new MyHolder(view);
    }

    @Override
    public void onBindViewHolder(@NonNull MyHolder myHolder, int i) {

        Items item=items.get(i);
        myHolder.title.setText(item.getName());
        Picasso.with(context).load(item.getPictures().getImg()).into(myHolder.picture);

        //因为roomid需要在请求直播房间视频的时候用，所以需要给itemclick，怎么给，这个地方设置了一个tag，非常巧
        myHolder.itemView.setTag(item.getId());
    }

    @Override
    public int getItemCount() {
        return items.size();
    }

    @Override
    public void onClick(View view) {
            if (mOnItemClickListener != null) {
                mOnItemClickListener.onItemClick((String) view.getTag());
            }
    }

    /**
     * 设置新数据
     * @param liveList
     */
    public void setItems(LiveList liveList) {
        items.clear();
        items.addAll(liveList.getData().getItems());
    }

    public void setmOnItemClickListener(OnItemClickListener mOnItemClickListener) {
        this.mOnItemClickListener = mOnItemClickListener;
    }

    public interface OnItemClickListener{
        void onItemClick(String id);
    }


    class MyHolder  extends RecyclerView.ViewHolder{

        ImageView picture;
        TextView title;

        public MyHolder(@NonNull View itemView) {
            super(itemView);
            picture=itemView.findViewById(R.id.picture);
            title=itemView.findViewById(R.id.title);
        }
    }
}
