<?php

123456; 123_456; // +
1234z6; 123456_; 123__456; // -

0x89Ab; 0x89_aB; // +
0x89zB; 0x89AB_; 0x_89AB; 0_x89AB; 0x89__AB; // -

1234.; 1234.e-0; 1234e+0; 1234e0; 1234e0.PHP_EOL; // +
1234._; 1234.e-; 1234e+; 1234e; // -

.1234; .12e0; // +
.12.0e0; .12e0.0; .12e0e0; // -

1.234e-10; 1.2_34e-1_0; // +
1.234e-_10; 1.234e_-10; 1.234_e-10; 1._234e-10; 1_.234e-10; // -
1.234e-+10; 1.234e-_+10; // -

01234567; 0_1234567; // +
012345678; // -

0...0; // 0. . .0
.0..0; // .0 . .0
0e+0+0e+0; // 0e+0 + 0e+0

;0#comment
;0//comment
;0/*comment*/;

?>
