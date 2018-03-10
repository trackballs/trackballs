;;; boxOfun

(day)
(set-track-name (_ "Box-O'-Fun"))
(set-author "Warren D Ober")
(start-time 340) 
(set-start-position 92 88)


;;1st pipe up.

(define pipe1 (pipe 111.2 127 0.8 114 127 1.5 0.5))
(set-wind pipe1 2.0 0.0)
(set-primary-color pipe1 0.1 0.1 0.1 1.0)
(pipe-connector 114 127 1.5 0.6)
(define pipe2 (pipe 114 127 1.5 113 127 8.6 0.5))
(set-wind pipe2 7.8 0.0)
(set-primary-color pipe2 0.1 0.1 0.1 1.0)

(add-flag 108 131 350 #t 0.1)
(add-flag 125 127 350 #t 0.1)
(add-flag 127 126 350 #t 0.1)
(add-flag 125 125 350 #t 0.1)
(add-flag 123 124 350 #t 0.1)


;;1st crosspipe.


(set-primary-color (pipe 113 125.8 9.0 113 114.2 9.0 0.5) 1.0 1.0 1.0 0.4)

(diamond 117 116)

;;2nd pipe.

(define pipe3 (pipe 123.2 127 6.4 125 127 7.0 0.5))
(set-wind pipe3 2.0 0.0)
(set-primary-color pipe3 0.1 0.1 0.1 0.9)
(pipe-connector 125 127 7.0 0.5)
(define pipe4 (pipe 125 127 7.0 125 123.2 8.4 0.5))
(set-wind pipe4 2.0 0.0)
(set-primary-color pipe4 0.1 0.1 0.1 0.9)

;;2nd crosspipe.


(set-primary-color (pipe 126 128.8 15.0 126 115 15.0 0.5) 1.0 1.0 1.0 0.4)




(trigger 114 130 2.0 (lambda () (fog)))
(trigger 118 130 2.0 (lambda () (day)))


(diamond 110 130)




;;More crosspipes

(set-primary-color (pipe 110.0 131.2 13.5 110.0 150.8 13.5 0.5) 1.0 1.0 1.0 0.4)

(set-primary-color (pipe 110.0 153.2 13.5 110.0 172.8 13.5 0.5) 1.0 1.0 1.0 0.4)

(set-primary-color (pipe 110.0 175.2 13.5 110.0 194.8 13.5 0.5) 1.0 1.0 1.0 0.4)

(set-primary-color (pipe 110.0 197.2 13.5 110.0 218.8 13.5 0.5) 1.0 1.0 1.0 0.4)

(set-primary-color (pipe 111.2 196.0 13.5 130.8 196.0 13.5 0.5) 1.0 1.0 1.0 0.4)

(set-primary-color (pipe 108.5 174.0 13.5 89.2  174.0 13.5 0.5) 1.0 1.0 1.0 0.4)

(set-primary-color (pipe 111.2 152.0 13.5 131.2 152.0 13.5 0.5) 1.0 1.0 1.0 0.4)

(set-primary-color (pipe 132.0 175.2 13.5 132.0 194.8 13.5 0.5) 1.0 1.0 1.0 0.4)

(set-primary-color (pipe 133.2 196.0 13.5 152.8 196.0 13.5 0.5) 1.0 1.0 1.0 0.4)

(set-primary-color (pipe 154.0 194.8 13.5 154.0 175.2 13.5 0.5) 1.0 1.0 1.0 0.4)


;; Goal!
(add-goal 132 174 #f "bx2")






