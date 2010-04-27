<?php
// $Id$
?>
$('#<?php echo $value['form_id'];?>').validate({<?php if($value['ajax_submit']) : ?>
	submitHandler: function(form) {
		jQuery(form).ajaxSubmit({
			<?php
        if(is_array($value['options'])){
          foreach($value['options'] as $key => $val){
            $t[] = $key. ':"' . $val .'"';
          }
          echo implode(',', $t);
        }
      ?>
		});
	}
  <?php endif; ?>});