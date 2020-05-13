# CSC10007-Project2

CSC10007 - Operating System Project #2

## Thông tin tổng quát

### Link GitHub

Chương trình của nhóm có thể xem online tại GitHub tại [đây](https://github.com/npkhang99/CSC10007-Project2).

### Thành viên nhóm

- Nguyễn Phúc Khang - 1712224

### Danh sách tính năng

- [x] Tạo một Linux kernel module sinh số ngẫu nhiên
- [x] Module tạo một character device để tiến trình từ user space có thể đọc số ngẫu nhiên được sinh ra
- [x] Hook system call `open` và `write`

## Cài đặt và chạy thử

### Điều kiện tiên quyết

Vì một số phiên bản kernel Linux sau này có thay đổi cách thức hoạt động và chương trình chỉ được test trên kernel phiên bản 3.13.0 (Ubuntu 12.04.5 LTS). Người dùng chạy thử nên dùng ở phiên bản kernel 3.x để được trải nghiệm tốt nhất, cho dù một số phiên bản đầu của kernel 4.x vẫn có thể chạy được bình thường.

Trên Debian / Ubuntu, chạy lệnh sau để cài đặt những gì cần thiết

```sh
$ sudo apt-get install build-essential linux-headers-$(uname -r)
```

### Linux kernel module


Mở terminal và chuyển thư mục hiện hành vào thư mục `rng`. Sau đó, chạy lệnh sau để biên dịch module

```sh
$ make
```

Sau khi module được biên dịch xong, bạn có thể thêm module vào hệ thống

```sh
sudo insmod driver.ko
```

Sau khi module được thêm vào hệ thống, bạn có thể xem log kernel hệ thống bằng lệnh

```sh
$ dmesg
```

Để chạy thử tiến trình ở user space đọc character driver, code có đính kèm theo một chương trình kiểm thử phần này. Đầu tiên bạn cần biên dịch với một lệnh đơn giản như sau

```sh
$ gcc test.c -o test
```

Sau đó, bạn nên chạy tệp thực thi dưới quyền `root` để có thể đọc được character driver một cách ổn định. Sau khi chạy được, bạn có thể nhấn phím Enter (hay Return) để đọc những số ngẫu được sinh ra trong module, nhấn một phím khác và sau đó Enter (hay Return) để thoát (hoặc `Ctrl-C`)

```sh
$ sudo ./test
```

Để gỡ cài đặt module, bạn chạy lệnh

```sh
$ sudo rmmod driver.ko
```

Nếu bạn muốn xem lại quá trình chạy của module, bạn có thể chạy `dmesg` để xem lại

```sh
$ dmesg
```

### Hook system call


Mở terminal và chuyển thư mục hiện hành vào thư mục `hook/`. Sau đó, chạy lệnh sau để biên dịch hook

```sh
$ make
```

Sau khi hook được biên dịch xong, bạn có thể thêm hook vào hệ thống

```sh
sudo insmod hook.ko
```

Lúc này, bạn có thể xem log kernel để bắt toàn bộ system call `open` và `write` được gọi bằng `dmesg`. Với những bản `dmesg` mới hơn, bạn có thể xem `dmesg` liên tục bằng cách gõ lệnh sau vào terminal

```sh
$ dmesg -w
```

Để xem `dmesg` liên tục trên những phiên bản cũ, bạn có thể chạy lệnh sau

```sh
$ watch "dmesg | tail -20"
```

Sau khi theo dõi hoàn tất và muốn gỡ cài đặt hook, bạn chạy lệnh sau

```sh
$ sudo rmmod hook.ko
```

**Lưu ý:** Vì hook sẽ liên tục in ra những thông tin về system call được gọi và hệ thống sẽ gọi system call rất rất nhiều lần nên sau khi cài đặt hook vào hệ thống, RAM và dung lượng ổ cứng sẽ bị ăn đi rất nhanh. Bạn nên ghi nhớ điều này để tranh thủ quan sát và gỡ cài đặt càng sớm càng tốt, không nên để hook chạy nền lâu.
