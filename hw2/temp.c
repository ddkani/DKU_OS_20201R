void *procedure_consumer(void *arg)
{
    so_t *so = arg;
    // ...
    while (TRUE)
    {
        pthread_mutex_lock(&so->lock);
        if (so->full == FALSE && so->eof)
        {
            // 작업이 종료되었습니다. 뮤택스 해제 후 종료
            break;
        }
        
        while(so->full == FALSE)
        {
            if (so->eof)
            {
                // 파일의 끝에 다다랐음. 스레드를 종료합니다.
                // producer, consumer 각각 신호 후 종료
                break;
            }
            pthread_cond_wait(&so->cond_cons, &so->lock);
        }
        buf = so->buf;
        if (buf == NULL) 
        { 
            // signal producer, 뮤택스 해제
            break;
        }
        linenum++;
        length += strlen(buf);
        // 문자열 통계

        so->full = FALSE;
        free(buf); // 동적 할당 & 복사한 데이터를 받았으므로 해제합니다.
        // signal producer, 뮤택스 해제
    }
}