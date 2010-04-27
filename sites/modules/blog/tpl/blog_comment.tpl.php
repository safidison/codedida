<?php
// $Id$
?>
<div id="blog_comment_<?php echo $comment->cid; ?>" class="blog_comment_view blog_comment_view_<?php echo $comment->type; ?> blog_comment_view_<?php echo $zebra; ?>">
  <p class="blog_comment_header">
    <strong><?php echo $comment->count;?>.</strong>
    <span class="blog_comment_name">
      <?php if($comment->site) { ?>
        <a rel="external" target="_blank" href="<?php echo $comment->site; ?>">
          <?php echo $comment->name; ?>
        </a>
        <?php if(!$comment->uid) : ?> (未登录) <?php endif;?>
      <?php }else{ ?>
        <?php echo user_name($comment); ?>
      <?php } ?>
    </span>
    - <?php echo format_date($comment->timestamp); ?>
    - <a href="#blog_comment_form" class="blog_comment_quote" alt="<?php echo $comment->cid; ?>">#引用</a>
  </p>
  <div class="blog_comment_body"><?php echo $comment->body; ?></div>
</div>