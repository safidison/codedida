<?php
/**
 * 默认区块模板 
 * $Id$
 */
?>

<div class="block block_<?php echo $block->region ?>_<?php echo $block->module ?>" id="block_<?php echo $block->region ?>_<?php echo $block->module ?>_<?php echo $block->delta ?>">
  <?php if($block->title) :?>
    <h3 class="block_title"><?php echo $block->title?></h3>
  <?php endif?>
  <div class="block_content">
    <?php echo $block->content?>
  </div>
</div>