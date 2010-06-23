<meta http-equiv="Content-Type" content="text/html; charset=utf-8" />
<script type="text/javascript" src="/misc/jquery.js"></script>
<script>
$(function() {
  $('.c').click(function() {
    window.parent.ck_insert_img($(this).attr('src'), $(this).attr('title'), $(this).attr('alt'), $(this).attr('href'));
    $(this).remove();
    return false;
  });
  $('.lists a').click(function() {return false;});
});
</script>

<?php for ($i = 0; $i < 10; ++$i) { ?>
  <a href="#" title="点击插入"><img width="85" height="85" class="c" src="http://eimgn.jiatx.com/news/2008_03/28/1206674544287_000.jpg" title="图片" href="http://eimgn.jiatx.com/news/2008_03/28/1206674544287_000.jpg"/></a>
<?php }; ?>