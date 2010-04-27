<?php 
  if($v){
    foreach($v as $module => $content){
      echo '<div class="front_block_'.$module.'">'.$content.'</div>';
    }
  }