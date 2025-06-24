# نمایشگر مدل سه‌بعدی  
این پروژه برای بارگذاری و نمایش اشیاء مدل سه‌بعدی قابل استفاده است.  
در این پروژه از Vulkan و API ویندوز (win32) برای محاسبه و رندر با استفاده از پردازنده گرافیکی (GPU) و پنجره ویندوز برای نمایش استفاده شده است.

## نحوه استفاده

کافیست برنامه اجرایی موجود در پوشه `release` را اجرا کرده و مدل سه‌بعدی `.obj` خود را داخل برنامه بکشید و رها کنید (Drag and Drop).

می‌توانید با نگه‌داشتن دکمه راست موس مدل را در محورهای X و Y بچرخانید و با اسکرول موس زوم کنید (بزرگ‌نمایی / کوچک‌نمایی).  
همچنین با کلیدهای W و S روی کیبورد می‌توانید در محور Z حرکت کنید.

## نویسندگان

- [@Kiarash0Salehi](https://www.github.com/Kiarash0Salehi)

## مجوز

Copyright 2025 Ki(y)arash Salehi

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the  Software ), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

## نشان‌ها

[![مجوز MIT](https://img.shields.io/badge/License-MIT-green.svg)](https://choosealicense.com/licenses/mit/)

## پشتیبانی

ایمیل: salehikiarash1389@gmail.com  
شماره: ‎+989368612901  
تلگرام: ‎+989368612901

## الزامات

برای اجرای این پروژه به کارت گرافیک و درایورهای آپدیت شده نیاز دارید 

برای اجرای این پروژه، باید الزامات زیر را برای کامپایل این پروژه نصب کنید

`cmake`: برای ایجاد پروژه

در ویندوز در صورت تمایل:
`MSVC`: برای کامپایل پروژه

`gcc & g++`: برای کامپایل پروژه

`VulkanSDK`: می توانید آن را از https://vulkan.lunarg.com/sdk/home دانلود کنید

## اجرای محلی

به پوشه پروژه بروید:

```bash
  cd vulkan1
```

برنامه را اجرا کنید:

```bash
  cd build/x64-Release
```

```bash
  ./Vulkan.exe
```

## متغیرهای محیطی

برای اجرای این پروژه، باید متغیرهای محیطی زیر را در فایل `.env` خود اضافه کنید:

`CXX` : مسیر به کامپایلر g++  
`CC`  : مسیر به کامپایلر gcc  
`VULKAN_SDK` : مسیر به پوشه نصب‌شده‌ی SDK ولکان

## 🚀 درباره من  
سلام! من کیارش صالحی هستم.  
برنامه‌نویس حرفه‌ای C/C++ با ۱.۵ سال تجربه و ۱ سال تجربه برنامه‌نویسی جاوا هستم.