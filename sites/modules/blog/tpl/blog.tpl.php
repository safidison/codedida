<?php
// $Id$
?>

<?php if($blog->position == 'page') { ?>
<div class="blog_page">
  <h2><?php echo $blog->title; ?></h2>
  <div class="blog_category">
    <?php if($blog->visit) : ?>
      <span>浏览 <?php echo $blog->visit; ?> 次</span>
    <?php endif;?>
    <?php if($blog->category) : ?>
      <span>分类 <?php echo $blog->category; ?></span>
    <?php endif;?>
    <?php if($blog->tags) : ?>
      <span>标签 <?php echo $blog->tags; ?></span>
    <?php endif;?>
  </div>

  <div class="blog_content">
    <?php echo $blog->body; ?>
  </div>

  <div class="blog_menu">
    <?php if($blog->next_link) : ?>
      <span class="blog_menu_next">前一篇：<?php echo $blog->next_link;?></span>
    <?php endif; ?>
    <?php if($blog->prev_link) : ?>
      <span class="blog_menu_prev">后一篇：<?php echo $blog->prev_link;?></span>
    <?php endif; ?>
  </div>

  <div class="blog_info">
    <?php echo user_name($blog);?> - 发布于 <?php echo format_date($blog->created); ?>
    <?php if($blog->updated) : ?>
    ，最近更新 <?php echo format_date($blog->updated); ?>
    <?php endif;?>
    
    <?php if($blog->is_comment) : ?>
      ，<a href="#blog_comment_form">添加新评论</a>
    <?php endif;?>
    <?php if($blog->trackback_url) : ?>
      ，<a rel="trackback" class="blog_trackback" href="<?php echo $blog->trackback_url ?>">通告地址</a>
    <?php endif;?>
    <?php if($blog->is_updated) : ?>
      ，<a href="<?php echo $blog->updated_url ?>">编辑日志</a>
    <?php endif;?>
  </div>
  
  <div id="blog_comment">
    <?php if($blog->comment_view) : ?>
      <h2>共 <?php echo $blog->comment_count; ?> 条评论</h2>
      <div class="blog_comment_list"><?php echo $blog->comment_view;?></div>
    <?php endif; ?>
    <?php echo $blog->comment_form;?>
  </div>
</div>

<?php }else{ ?>

<div class="blog_summary">
  <h2><a href="<?php echo $blog->url?>"><?php echo $blog->title; ?></a></h2>
  <div class="blog_category">
    <?php if($blog->visit) : ?>
      <span>浏览 <?php echo $blog->visit; ?> 次</span>
    <?php endif;?>
    <?php if($blog->category) : ?>
      <span>分类 <?php echo $blog->category; ?></span>
    <?php endif;?>
    <?php if($blog->tags) : ?>
      <span>标签 <?php echo $blog->tags; ?></span>
    <?php endif;?>
  </div>

  <div class="blog_content">
    <?php echo $blog->summary; ?>
  </div>
  
  <div class="blog_info">
    <?php echo user_name($blog);?> 发布于 <?php echo format_date($blog->created); ?>
      ，<a href="<?php echo $blog->url?>">查看更多</a>
    <?php if($blog->is_comment) : ?>
      ，<a href="<?php echo $blog->url?>#blog_comment_form">添加新评论</a>
    <?php endif;?>
    <?php if($blog->comment_count) : ?>
      ，共 <?php echo $blog->comment_count; ?> 条评论
    <?php endif; ?>
    <?php if($blog->is_updated) : ?>
      ，<a href="<?php echo $blog->updated_url ?>">编辑日志</a>
    <?php endif;?>
  </div>
</div>
<?php }; ?>