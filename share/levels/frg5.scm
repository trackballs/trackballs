(set-track-name "Upside Down")
(set-author "Ulrik Enstad")

(set-start-position 172 172)

(new-baby 184.5 185.5)
(new-baby 186.5 183.5)
(new-baby 182.5 187.5)
(new-baby 182.5 184.5)

(define pipe1 (pipe 188.5 188.5 2.1 189.0 189.0 2.1 0.4))
(define pipe2 (pipe-connector 189.0 189.0 2.1 0.4))
(define pipe3 (pipe 189.0 189.0 2.1 189.5 189.5 2.7 0.4))
(define pipe4 (pipe-connector 189.5 189.5 2.7 0.4))
(define pipe5 (pipe 189.5 189.5 2.7 189.5 189.4 3.6 0.4))
(define pipe6 (pipe-connector 189.5 189.4 3.6 0.4))
(define pipe7 (pipe 189.5 189.5 3.6 189.5 188.6 4.2 0.4))

(set-wind pipe1 16.0 0.0)
(set-wind pipe2 16.0 0.0)
(set-wind pipe3 16.0 0.0)
(set-wind pipe4 16.0 0.0)
(set-wind pipe5 16.0 0.0)
(set-wind pipe6 16.0 0.0)
(set-wind pipe7 16.0 0.0)

(diamond 190.0 182.0)

(cond
 ((= (difficulty) *easy*)
         (define speed .4)
 )
 ((= (difficulty) *normal*)
         (define speed .3)
 )
 ((= (difficulty) *hard*)
         (define speed .2)
 )
)

(add-cyclic-platform 204 191 204 191 4.9 6.4 1. speed)
(add-cyclic-platform 204 190 204 190 4.9 6.4 6. speed)
(add-cyclic-platform 205 191 205 191 4.9 6.4 3. speed)
(add-cyclic-platform 205 190 205 190 4.9 6.4 5. speed)
(add-cyclic-platform 206 191 206 191 4.9 6.4 2. speed)
(add-cyclic-platform 206 190 206 190 4.9 6.4 4. speed)

(add-cyclic-platform 207 194 207 194 4.9 6.4 1. speed)
(add-cyclic-platform 208 194 208 194 4.9 6.4 4. speed)
(add-cyclic-platform 207 195 207 195 4.9 6.4 2. speed)
(add-cyclic-platform 208 195 208 195 4.9 6.4 3. speed)
(add-cyclic-platform 207 196 207 196 4.9 6.4 6. speed)
(add-cyclic-platform 208 196 208 196 4.9 6.4 5. speed)

(set-wind (pipe 208.0 199.0 5.4 208.0 201.0 6.1 0.4) 8.0 0.0)
(set-wind (pipe-connector 208.0 201.0 6.1 0.4) 8.0 0.0)
(set-wind (pipe 208.0 201.0 6.1 208.1 201.0 7.7 0.4) 8.0 0.0)

(add-goal 211 201 #t "frg6")
(add-goal 211 200 #t "frg6")

(jump 0.0)

(add-flag 208 195 30 0 1.5)
(add-flag 206 190 30 0 1.5)
(add-flag 200 188 30 0 0.5)
(add-flag 189 188 30 0 0.5)

(cond
 ((= (difficulty) *easy*)
         (start-time 120)
 )
 ((= (difficulty) *normal*)
         (start-time 100)
 )
 ((= (difficulty) *hard*)
         (start-time 60)
 )
)

(restart-time 0)



