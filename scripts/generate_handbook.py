#!/usr/bin/env python3
"""
MRT Programlama Dili El Kitabı (v0.2.0) PDF Oluşturucu
ReportLab kütüphanesi kullanılarak profesyonel, yayın kalitesinde PDF üretir.
"""

import os
import sys
from reportlab.lib.pagesizes import A4
from reportlab.lib import colors
from reportlab.lib.units import mm, cm
from reportlab.lib.styles import getSampleStyleSheet, ParagraphStyle
from reportlab.platypus import (
    SimpleDocTemplate, Paragraph, Spacer, Table, TableStyle, PageBreak, KeepTogether, HRFlowable, Preformatted
)
from reportlab.pdfgen import canvas
from reportlab.pdfbase import pdfmetrics
from reportlab.pdfbase.ttfonts import TTFont
from reportlab.pdfbase.pdfmetrics import registerFontFamily

# Fontları kaydet (DejaVu Sans ve DejaVu Sans Mono - Tam Türkçe Desteği)
FONT_DIR = "/usr/share/fonts/truetype/dejavu"
pdfmetrics.registerFont(TTFont("DejaVuSans", f"{FONT_DIR}/DejaVuSans.ttf"))
pdfmetrics.registerFont(TTFont("DejaVuSans-Bold", f"{FONT_DIR}/DejaVuSans-Bold.ttf"))
pdfmetrics.registerFont(TTFont("DejaVuSans-Oblique", f"{FONT_DIR}/DejaVuSans-Oblique.ttf" if os.path.exists(f"{FONT_DIR}/DejaVuSans-Oblique.ttf") else f"{FONT_DIR}/DejaVuSans.ttf"))
pdfmetrics.registerFont(TTFont("DejaVuSansMono", f"{FONT_DIR}/DejaVuSansMono.ttf"))
pdfmetrics.registerFont(TTFont("DejaVuSansMono-Bold", f"{FONT_DIR}/DejaVuSansMono-Bold.ttf"))

registerFontFamily(
    "DejaVuSans",
    normal="DejaVuSans",
    bold="DejaVuSans-Bold",
    italic="DejaVuSans-Oblique",
    boldItalic="DejaVuSans-Bold"
)

registerFontFamily(
    "DejaVuSansMono",
    normal="DejaVuSansMono",
    bold="DejaVuSansMono-Bold",
    italic="DejaVuSansMono",
    boldItalic="DejaVuSansMono-Bold"
)

# Renk Paleti
COLOR_PRIMARY = colors.HexColor("#1A237E")      # Koyu İndigo
COLOR_SECONDARY = colors.HexColor("#283593")    # İndigo
COLOR_ACCENT = colors.HexColor("#E65100")       # Kehribar/Turuncu
COLOR_DARK = colors.HexColor("#212121")         # Gövde Metni
COLOR_MUTED = colors.HexColor("#546E7A")        # İkincil Metin
COLOR_BG_LIGHT = colors.HexColor("#F8F9FA")     # Açık Arka Plan
COLOR_BG_CODE = colors.HexColor("#F0F4F8")      # Kod Kutusu Arka Planı
COLOR_BORDER = colors.HexColor("#CFD8DC")       # İnce Çizgi

class NumberedCanvas(canvas.Canvas):
    """İki geçişli sayfa numaralandırma ve koşan başlık/altlık tuvali"""
    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)
        self._saved_page_states = []

    def showPage(self):
        self._saved_page_states.append(dict(self.__dict__))
        self._startPage()

    def save(self):
        num_pages = len(self._saved_page_states)
        for state in self._saved_page_states:
            self.__dict__.update(state)
            self.draw_page_decorations(num_pages)
            super().showPage()
        super().save()

    def draw_page_decorations(self, total_pages):
        if self._pageNumber == 1:
            return  # Kapak sayfasında üst ve alt bilgi gösterilmez

        self.saveState()
        self.setFont("DejaVuSans", 8.5)
        self.setFillColor(COLOR_MUTED)

        # Üst Bilgi (Header)
        self.drawString(20 * mm, 285 * mm, "MRT Programlama Dili — Resmi El Kitabı (v0.2.0)")
        self.drawRightString(190 * mm, 285 * mm, "MRT & MRT Studio")
        self.setStrokeColor(COLOR_BORDER)
        self.setLineWidth(0.5)
        self.line(20 * mm, 282 * mm, 190 * mm, 282 * mm)

        # Alt Bilgi (Footer)
        page_str = f"Sayfa {self._pageNumber} / {total_pages}"
        self.line(20 * mm, 15 * mm, 190 * mm, 15 * mm)
        self.drawString(20 * mm, 11 * mm, "© 2026 Murat Özçelik — MIT Lisansı ile açık kaynak")
        self.drawRightString(190 * mm, 11 * mm, page_str)

        self.restoreState()


def create_styles():
    styles = getSampleStyleSheet()

    styles.add(ParagraphStyle(
        "CoverTitle",
        fontName="DejaVuSans-Bold",
        fontSize=30,
        leading=38,
        textColor=COLOR_PRIMARY,
        alignment=0,
        spaceAfter=12
    ))

    styles.add(ParagraphStyle(
        "CoverSubtitle",
        fontName="DejaVuSans",
        fontSize=15,
        leading=22,
        textColor=COLOR_ACCENT,
        alignment=0,
        spaceAfter=24
    ))

    styles.add(ParagraphStyle(
        "CoverMeta",
        fontName="DejaVuSans",
        fontSize=10,
        leading=16,
        textColor=COLOR_MUTED,
        spaceAfter=6
    ))

    styles.add(ParagraphStyle(
        "SectionHeading",
        fontName="DejaVuSans-Bold",
        fontSize=18,
        leading=24,
        textColor=COLOR_PRIMARY,
        spaceBefore=18,
        spaceAfter=10,
        keepWithNext=True
    ))

    styles.add(ParagraphStyle(
        "SubSectionHeading",
        fontName="DejaVuSans-Bold",
        fontSize=13,
        leading=18,
        textColor=COLOR_SECONDARY,
        spaceBefore=14,
        spaceAfter=6,
        keepWithNext=True
    ))

    styles.add(ParagraphStyle(
        "Body",
        fontName="DejaVuSans",
        fontSize=10,
        leading=15,
        textColor=COLOR_DARK,
        spaceAfter=8
    ))

    styles.add(ParagraphStyle(
        "BodyBold",
        fontName="DejaVuSans-Bold",
        fontSize=10,
        leading=15,
        textColor=COLOR_DARK,
        spaceAfter=8
    ))

    styles.add(ParagraphStyle(
        "BulletItem",
        fontName="DejaVuSans",
        fontSize=10,
        leading=15,
        textColor=COLOR_DARK,
        leftIndent=14,
        firstLineIndent=-10,
        spaceAfter=4
    ))

    styles.add(ParagraphStyle(
        "CodeText",
        fontName="DejaVuSansMono",
        fontSize=8.5,
        leading=12,
        textColor=COLOR_DARK
    ))

    styles.add(ParagraphStyle(
        "TableHead",
        fontName="DejaVuSans-Bold",
        fontSize=9,
        leading=12,
        textColor=colors.white,
        alignment=0
    ))

    styles.add(ParagraphStyle(
        "TableCell",
        fontName="DejaVuSans",
        fontSize=8.5,
        leading=12,
        textColor=COLOR_DARK
    ))

    styles.add(ParagraphStyle(
        "TableCellCode",
        fontName="DejaVuSansMono-Bold",
        fontSize=8.5,
        leading=12,
        textColor=COLOR_PRIMARY
    ))

    styles.add(ParagraphStyle(
        "NoteBox",
        fontName="DejaVuSans",
        fontSize=9.5,
        leading=14,
        textColor=COLOR_PRIMARY,
        leftIndent=10,
        rightIndent=10,
        spaceBefore=6,
        spaceAfter=6
    ))

    return styles


def make_code_block(code_str, styles):
    """Kod bloğu oluşturur: açık gri arka plan, kenarlık, monospace yazı tipi"""
    p = Preformatted(code_str.strip(), styles["CodeText"])
    t = Table([[p]], colWidths=[170 * mm])
    t.setStyle(TableStyle([
        ('BACKGROUND', (0, 0), (-1, -1), COLOR_BG_CODE),
        ('BOX', (0, 0), (-1, -1), 0.75, COLOR_BORDER),
        ('LEFTPADDING', (0, 0), (-1, -1), 10),
        ('RIGHTPADDING', (0, 0), (-1, -1), 10),
        ('TOPPADDING', (0, 0), (-1, -1), 8),
        ('BOTTOMPADDING', (0, 0), (-1, -1), 8),
    ]))
    return t


def make_note_box(title, text, styles):
    """Önemli not kutucuğu"""
    content = [
        Paragraph(f"<b>{title}:</b> {text}", styles["NoteBox"])
    ]
    t = Table([[content]], colWidths=[170 * mm])
    t.setStyle(TableStyle([
        ('BACKGROUND', (0, 0), (-1, -1), colors.HexColor("#FFF8E1")),
        ('BOX', (0, 0), (-1, -1), 1, COLOR_ACCENT),
        ('LEFTPADDING', (0, 0), (-1, -1), 12),
        ('RIGHTPADDING', (0, 0), (-1, -1), 12),
        ('TOPPADDING', (0, 0), (-1, -1), 8),
        ('BOTTOMPADDING', (0, 0), (-1, -1), 8),
    ]))
    return t


def build_pdf(filename="docs/MRT_El_Kitabi.pdf"):
    os.makedirs(os.path.dirname(filename), exist_ok=True)
    doc = SimpleDocTemplate(
        filename,
        pagesize=A4,
        leftMargin=20 * mm,
        rightMargin=20 * mm,
        topMargin=25 * mm,
        bottomMargin=22 * mm
    )

    styles = create_styles()
    story = []

    # =========================================================================
    # KAPAK SAYFASI
    # =========================================================================
    story.append(Spacer(1, 20 * mm))
    story.append(HRFlowable(width="100%", thickness=6, color=COLOR_ACCENT, spaceAfter=20))
    story.append(Paragraph("MRT PROGRAMLAMA DİLİ", styles["CoverTitle"]))
    story.append(Paragraph("Resmi Başvuru ve Kullanım El Kitabı", styles["CoverSubtitle"]))
    story.append(Paragraph("Sürüm 0.2.0 • Saf C ile Sıfırdan Geliştirilen Bağımsız Programlama Dili ve Yerel Linux IDE'si", styles["Body"]))
    story.append(HRFlowable(width="100%", thickness=1, color=COLOR_BORDER, spaceBefore=20, spaceAfter=30))

    meta_table_data = [
        [Paragraph("<b>Dil Sürümü:</b>", styles["TableCell"]), Paragraph("MRT 0.2.0", styles["TableCell"])],
        [Paragraph("<b>IDE Sürümü:</b>", styles["TableCell"]), Paragraph("MRT Studio 0.2.0", styles["TableCell"])],
        [Paragraph("<b>Mimari:</b>", styles["TableCell"]), Paragraph("Saf Standart C (C11/C17), AST Tree-Walking Yorumlayıcı", styles["TableCell"])],
        [Paragraph("<b>Grafik Arayüz:</b>", styles["TableCell"]), Paragraph("GTK4 + GtkSourceView 5 (Hafif, Yerel Linux)", styles["TableCell"])],
        [Paragraph("<b>Geliştirici:</b>", styles["TableCell"]), Paragraph("Murat Özçelik", styles["TableCell"])],
        [Paragraph("<b>Kaynak Kodu:</b>", styles["TableCell"]), Paragraph("https://github.com/MOzcelik14/MRT", styles["TableCell"])],
        [Paragraph("<b>Lisans:</b>", styles["TableCell"]), Paragraph("MIT Açık Kaynak Lisansı", styles["TableCell"])],
        [Paragraph("<b>Tarih:</b>", styles["TableCell"]), Paragraph("Ekim 2026", styles["TableCell"])],
    ]
    meta_table = Table(meta_table_data, colWidths=[40 * mm, 130 * mm])
    meta_table.setStyle(TableStyle([
        ('BACKGROUND', (0, 0), (-1, -1), COLOR_BG_LIGHT),
        ('BOX', (0, 0), (-1, -1), 1, COLOR_BORDER),
        ('INNERGRID', (0, 0), (-1, -1), 0.5, COLOR_BORDER),
        ('TOPPADDING', (0, 0), (-1, -1), 6),
        ('BOTTOMPADDING', (0, 0), (-1, -1), 6),
        ('LEFTPADDING', (0, 0), (-1, -1), 10),
        ('RIGHTPADDING', (0, 0), (-1, -1), 10),
    ]))
    story.append(meta_table)

    story.append(Spacer(1, 40 * mm))
    story.append(Paragraph("<i>\"Bu proje basit bir parser demosu değil; temiz mimarili, genişletilebilir ve sıfır bağımlılıklı gerçek bir programlama dili implementasyonudur.\"</i>", styles["CoverMeta"]))
    story.append(PageBreak())

    # =========================================================================
    # İÇİNDEKİLER / ÖZET
    # =========================================================================
    story.append(Paragraph("İçindekiler Tablosu", styles["SectionHeading"]))
    story.append(HRFlowable(width="100%", thickness=1.5, color=COLOR_PRIMARY, spaceAfter=14))

    toc_items = [
        ("1. Giriş ve Dilin Temel Felsefesi", "MRT'nin çıkış noktası, sıfır harici bağımlılık ilkesi ve temiz C mimarisi."),
        ("2. Kurulum ve Hızlı Başlangıç", "Derleme, test suit, sisteme kurulum, REPL ve komut satırı argümanları."),
        ("3. MRT 0.2.0 Sözdizimi ve Veri Tipleri", "Değişkenler (var), temel veri tipleri (yes, no, none), dizgeler ve blok yorumlar."),
        ("4. Operatörler ve İfadeler", "Aritmetik, mantıksal, karşılaştırma operatörleri ve öncelik sıralaması."),
        ("5. Konsol Çıktısı: 'say' Deyimi", "say <ifade> deyiminin doğası, otomatik satır sonu ve birinci sınıf dil yapısı."),
        ("6. Kontrol Akışı (when, otherwise, repeat)", "when/otherwise blokları, repeat döngüsü, break ve continue akış kontrolü."),
        ("7. Görevler (Tasks / Fonksiyonlar)", "task tanımları, give deyimi, sözcüksel kapsam (lexical scope) ve özyineleme."),
        ("8. Yerleşik Standart Kütüphane", "read, toNumber, toText, typeOf, length ve clock fonksiyonlarının kullanımı ve tipleri."),
        ("9. Hata Yönetimi ve Tanılama", "SyntaxError, NameError, TypeError, RuntimeError ve imleçli görsel hata işaretçileri."),
        ("10. MRT Studio IDE Kılavuzu", "GTK4 yerel arayüzü, çift dilli destek, komut paleti, sembol ağacı ve kurtarma."),
        ("11. Dil Mimarisi ve Gelecek Yol Haritası", "Lexer -> Parser -> AST -> Interpreter ardışık düzeni ve gelecek hedefler."),
        ("12. Hızlı Başvuru Kartı (Cheat Sheet)", "Tek bakışta sözdizimi, anahtar sözcükler ve kısayollar tablosu."),
    ]

    for title, desc in toc_items:
        story.append(Paragraph(f"<b>{title}</b>", styles["BodyBold"]))
        story.append(Paragraph(desc, styles["BulletItem"]))
        story.append(Spacer(1, 2 * mm))

    story.append(PageBreak())

    # =========================================================================
    # BÖLÜM 1: GİRİŞ VE DİLİN FELSEFESİ
    # =========================================================================
    story.append(Paragraph("1. Giriş ve Dilin Temel Felsefesi", styles["SectionHeading"]))
    story.append(HRFlowable(width="100%", thickness=1.5, color=COLOR_PRIMARY, spaceAfter=10))

    story.append(Paragraph(
        "<b>MRT</b>, standart C (C11/C17) diliyle sıfırdan geliştirilmiş modern, dinamik tipli ve temiz mimarili bir programlama dilidir. "
        "MRT projesinin temel amacı; harici kod oluşturuculara (Flex, Bison, ANTLR vb.) ihtiyaç duymadan, baştan sona saf C standart kütüphanesi "
        "kullanılarak endüstri standartlarında bir dil çekirdeğinin nasıl inşa edilebileceğini somutlaştırmaktır.",
        styles["Body"]
    ))

    story.append(Paragraph("Temel Tasarım İlkeleri:", styles["SubSectionHeading"]))
    story.append(Paragraph("• <b>Sıfır Dış Bağımlılık (Standard C Only):</b> Dil yorumlayıcısı yalnızca ANSI/ISO C11 standart kütüphanesini kullanır.", styles["BulletItem"]))
    story.append(Paragraph("• <b>El Yapımı Ön Yüz (Handwritten Frontend):</b> Lexer (sözcüksel analiz) ve Recursive Descent Parser tamamen özel olarak kodlanmıştır.", styles["BulletItem"]))
    story.append(Paragraph("• <b>Ayrık Katman Mimarisi:</b> Dil ön yüzü (AST) ile çalışma zamanı (Interpreter) birbirinden kesin sınırlarla ayrılmıştır.", styles["BulletItem"]))
    story.append(Paragraph("• <b>Mutlak Bellek Güvenliği:</b> Referans sayımlı bellek yönetimi uygulanmış olup, AddressSanitizer (ASan) ve LeakSanitizer (LSan) ile <b>0 bayt bellek sızıntısı</b> doğrulanmıştır.", styles["BulletItem"]))
    story.append(Paragraph("• <b>Özgün Sözdizimi Kimliği:</b> MRT 0.2.0 sürümüyle birlikte jenerik kalıplar bırakılmış; <code>var</code>, <code>task</code>, <code>give</code>, <code>say</code>, <code>when</code>, <code>repeat</code> gibi özgün anahtar sözcükler benimsenmiştir.", styles["BulletItem"]))

    story.append(Spacer(1, 4 * mm))
    story.append(make_note_box(
        "MRT 0.2.0 Dönüm Noktası",
        "MRT, 0.1 sürümündeki genel JavaScript/Python benzeri sözdizimini geride bırakarak kendi dil kimliğini kazanmıştır. "
        "Bu dokümanda yer alan tüm örnekler ve açıklamalar güncel MRT 0.2.0 sözdizimini esas almaktadır.",
        styles
    ))

    # =========================================================================
    # BÖLÜM 2: KURULUM VE HIZLI BAŞLANGIÇ
    # =========================================================================
    story.append(Paragraph("2. Kurulum ve Hızlı Başlangıç", styles["SectionHeading"]))
    story.append(HRFlowable(width="100%", thickness=1.5, color=COLOR_PRIMARY, spaceAfter=10))

    story.append(Paragraph("Gereksinimler:", styles["SubSectionHeading"]))
    story.append(Paragraph("• C Derleyicisi: <b>GCC (11+)</b> veya <b>Clang (13+)</b>", styles["BulletItem"]))
    story.append(Paragraph("• Derleme Aracı: <b>GNU Make</b>", styles["BulletItem"]))
    story.append(Paragraph("• İşletim Sistemi: Modern Linux dağıtımları (Ubuntu, Debian, Fedora, Arch, vb.)", styles["BulletItem"]))

    story.append(Paragraph("Derleme Komutları:", styles["SubSectionHeading"]))
    story.append(make_code_block("""# 1. Depoyu klonlayın
git clone https://github.com/MOzcelik14/MRT.git
cd MRT

# 2. MRT yorumlayıcısını derleyin
make

# 3. Test paketini çalıştırın (AddressSanitizer aktif)
make test

# 4. MRT Studio IDE'sini derleyin
make studio

# 5. Sisteme kullanıcı düzeyinde (~/.local/bin) kurun
make install PREFIX=~/.local
""", styles))

    story.append(Paragraph("İlk Programınız: Merhaba Dünya!", styles["SubSectionHeading"]))
    story.append(Paragraph("MRT kaynak dosyaları <code>.mrt</code> uzantısını taşır. Favori metin editörünüzle <code>merhaba.mrt</code> adında bir dosya oluşturun:", styles["Body"]))
    story.append(make_code_block("""// merhaba.mrt
say "Merhaba Dünya! MRT 0.2.0 çalışıyor."
""", styles))

    story.append(Paragraph("Programı terminalden çalıştırmak için:", styles["Body"]))
    story.append(make_code_block("""mrt merhaba.mrt""", styles))

    story.append(Paragraph("İnteraktif REPL Modu:", styles["SubSectionHeading"]))
    story.append(Paragraph("MRT, dosya adı vermeksizin çalıştırıldığında anlık deneme yapabileceğiniz interaktif REPL moduna geçer:", styles["Body"]))
    story.append(make_code_block("""$ mrt
MRT 0.2.0 Interactive REPL
Type 'exit' to quit.

mrt> var x = 15 * 3
mrt> say x
45
mrt> say toText(x) + " elma"
45 elma
mrt> exit
""", styles))

    story.append(Paragraph("Komut Satırı Bayrakları:", styles["SubSectionHeading"]))
    story.append(Paragraph("• <code>mrt &lt;dosya.mrt&gt;</code>: Kaynak dosyayı yorumlar ve çalıştırır.", styles["BulletItem"]))
    story.append(Paragraph("• <code>mrt --tokens &lt;dosya.mrt&gt;</code>: Sözcüksel analiz sonucundaki token akışını döker.", styles["BulletItem"]))
    story.append(Paragraph("• <code>mrt --ast &lt;dosya.mrt&gt;</code>: Oluşturulan Soyut Sözdizim Ağacını (AST) hiyerarşik olarak yazdırır.", styles["BulletItem"]))
    story.append(Paragraph("• <code>mrt --version</code>: MRT sürüm numarasını görüntüler.", styles["BulletItem"]))
    story.append(Paragraph("• <code>mrt --help</code>: Kullanım yardımını gösterir.", styles["BulletItem"]))

    story.append(PageBreak())

    # =========================================================================
    # BÖLÜM 3: SÖZDİZİMİ VE VERİ TİPLERİ
    # =========================================================================
    story.append(Paragraph("3. MRT 0.2.0 Sözdizimi ve Veri Tipleri", styles["SectionHeading"]))
    story.append(HRFlowable(width="100%", thickness=1.5, color=COLOR_PRIMARY, spaceAfter=10))

    story.append(Paragraph("Yorum Satırları:", styles["SubSectionHeading"]))
    story.append(Paragraph("MRT'de tek satırlık ve çok satırlı blok yorumlar desteklenir:", styles["Body"]))
    story.append(make_code_block("""// Tek satırlık açıklama
var pi = 3.14

/*
   Bu bir blok yorumdur.
   Birden fazla satıra yayılabilir.
*/
var yaricap = 5
""", styles))

    story.append(Paragraph("Değişkenler (Variables):", styles["SubSectionHeading"]))
    story.append(Paragraph(
        "Değişken tanımlamak için <code>var</code> anahtar sözcüğü kullanılır. "
        "MRT'de noktalı virgül (<code>;</code>) isteğe bağlıdır; yeni satırlar deyim sonlandırıcı kabul edilir:",
        styles["Body"]
    ))
    story.append(make_code_block("""var isim = "Murat"
var yas = 22
yas = yas + 1   // Değer yeniden atama
""", styles))

    story.append(Paragraph("Veri Tipleri (Data Types):", styles["SubSectionHeading"]))

    types_data = [
        [Paragraph("Tip", styles["TableHead"]), Paragraph("Örnek Değerler", styles["TableHead"]), Paragraph("Açıklama", styles["TableHead"])],
        [Paragraph("integer", styles["TableCellCode"]), Paragraph("10, -5, 0, 1000000", styles["TableCell"]), Paragraph("64-bit işaretli tamsayı (int64_t).", styles["TableCell"])],
        [Paragraph("float", styles["TableCellCode"]), Paragraph("3.14, -0.05, 2.0", styles["TableCell"]), Paragraph("64-bit çift duyarlıklı kayan noktalı sayı (double).", styles["TableCell"])],
        [Paragraph("string", styles["TableCellCode"]), Paragraph('"Merhaba\\nDünya"', styles["TableCell"]), Paragraph("Dinamik boyutlu UTF-8 metin dizgisi.", styles["TableCell"])],
        [Paragraph("boolean", styles["TableCellCode"]), Paragraph("yes, no", styles["TableCell"]), Paragraph("Mantıksal doğruluk (yes) veya yanlışlık (no).", styles["TableCell"])],
        [Paragraph("none", styles["TableCellCode"]), Paragraph("none", styles["TableCell"]), Paragraph("Boşluk/yokluk değeri (null/nil dengi).", styles["TableCell"])],
        [Paragraph("function", styles["TableCellCode"]), Paragraph("task kare(x) { ... }", styles["TableCell"]), Paragraph("Birinci sınıf fonksiyon/görev nesnesi.", styles["TableCell"])],
    ]
    types_table = Table(types_data, colWidths=[28 * mm, 45 * mm, 97 * mm])
    types_table.setStyle(TableStyle([
        ('BACKGROUND', (0, 0), (-1, 0), COLOR_PRIMARY),
        ('BOX', (0, 0), (-1, -1), 1, COLOR_BORDER),
        ('INNERGRID', (0, 0), (-1, -1), 0.5, COLOR_BORDER),
        ('ROWBACKGROUNDS', (0, 1), (-1, -1), [colors.white, COLOR_BG_LIGHT]),
        ('TOPPADDING', (0, 0), (-1, -1), 5),
        ('BOTTOMPADDING', (0, 0), (-1, -1), 5),
        ('LEFTPADDING', (0, 0), (-1, -1), 8),
        ('RIGHTPADDING', (0, 0), (-1, -1), 8),
    ]))
    story.append(types_table)

    story.append(Spacer(1, 4 * mm))
    story.append(Paragraph("Dizgelerde Kaçış Karakterleri (Escape Sequences):", styles["SubSectionHeading"]))
    story.append(Paragraph("Dizgeler çift tırnak (<code>\"...\"</code>) ile çevrelenir ve şu kaçış karakterlerini destekler:", styles["Body"]))
    story.append(Paragraph("• <code>\\n</code> : Yeni satır (newline)", styles["BulletItem"]))
    story.append(Paragraph("• <code>\\t</code> : Sekme boşluğu (tab)", styles["BulletItem"]))
    story.append(Paragraph("• <code>\\\"</code> : Çift tırnak karakteri", styles["BulletItem"]))
    story.append(Paragraph("• <code>\\\\</code> : Ters eğik çizgi (backslash)", styles["BulletItem"]))

    # =========================================================================
    # BÖLÜM 4: OPERATÖRLER VE İFADELER
    # =========================================================================
    story.append(Paragraph("4. Operatörler ve İfadeler", styles["SectionHeading"]))
    story.append(HRFlowable(width="100%", thickness=1.5, color=COLOR_PRIMARY, spaceAfter=10))

    story.append(Paragraph(
        "MRT, zengin aritmetik, karşılaştırma ve mantıksal operatörlere sahiptir. "
        "Operatörler standart matematiksel öncelik kurallarına (precedence climbing) göre kesin bir hiyerarşide değerlendirilir:",
        styles["Body"]
    ))

    prec_data = [
        [Paragraph("Öncelik", styles["TableHead"]), Paragraph("Operatör", styles["TableHead"]), Paragraph("Tür", styles["TableHead"]), Paragraph("Yön", styles["TableHead"])],
        [Paragraph("1 (En Düşük)", styles["TableCell"]), Paragraph("=", styles["TableCellCode"]), Paragraph("Atama", styles["TableCell"]), Paragraph("Sağdan Sola", styles["TableCell"])],
        [Paragraph("2", styles["TableCell"]), Paragraph("or", styles["TableCellCode"]), Paragraph("Mantıksal VEYA", styles["TableCell"]), Paragraph("Soldan Sağa", styles["TableCell"])],
        [Paragraph("3", styles["TableCell"]), Paragraph("and", styles["TableCellCode"]), Paragraph("Mantıksal VE", styles["TableCell"]), Paragraph("Soldan Sağa", styles["TableCell"])],
        [Paragraph("4", styles["TableCell"]), Paragraph("==, !=", styles["TableCellCode"]), Paragraph("Eşitlik / Eşitsizlik", styles["TableCell"]), Paragraph("Soldan Sağa", styles["TableCell"])],
        [Paragraph("5", styles["TableCell"]), Paragraph("<, <=, >, >=", styles["TableCellCode"]), Paragraph("Büyüklük / Küçüklük", styles["TableCell"]), Paragraph("Soldan Sağa", styles["TableCell"])],
        [Paragraph("6", styles["TableCell"]), Paragraph("+, -", styles["TableCellCode"]), Paragraph("Toplama / Çıkarma", styles["TableCell"]), Paragraph("Soldan Sağa", styles["TableCell"])],
        [Paragraph("7", styles["TableCell"]), Paragraph("*, /, %", styles["TableCellCode"]), Paragraph("Çarpma / Bölme / Mod", styles["TableCell"]), Paragraph("Soldan Sağa", styles["TableCell"])],
        [Paragraph("8", styles["TableCell"]), Paragraph("-, not", styles["TableCellCode"]), Paragraph("Tekil Eksi / Değil", styles["TableCell"]), Paragraph("Önek (Prefix)", styles["TableCell"])],
        [Paragraph("9", styles["TableCell"]), Paragraph("()", styles["TableCellCode"]), Paragraph("Fonksiyon Çağrısı", styles["TableCell"]), Paragraph("Sonek (Postfix)", styles["TableCell"])],
        [Paragraph("10 (En Yüksek)", styles["TableCell"]), Paragraph("Değer, (ifade)", styles["TableCellCode"]), Paragraph("Temel / Parantez", styles["TableCell"]), Paragraph("—", styles["TableCell"])],
    ]
    prec_table = Table(prec_data, colWidths=[28 * mm, 32 * mm, 65 * mm, 45 * mm])
    prec_table.setStyle(TableStyle([
        ('BACKGROUND', (0, 0), (-1, 0), COLOR_SECONDARY),
        ('BOX', (0, 0), (-1, -1), 1, COLOR_BORDER),
        ('INNERGRID', (0, 0), (-1, -1), 0.5, COLOR_BORDER),
        ('ROWBACKGROUNDS', (0, 1), (-1, -1), [colors.white, COLOR_BG_LIGHT]),
        ('TOPPADDING', (0, 0), (-1, -1), 4),
        ('BOTTOMPADDING', (0, 0), (-1, -1), 4),
        ('LEFTPADDING', (0, 0), (-1, -1), 8),
        ('RIGHTPADDING', (0, 0), (-1, -1), 8),
    ]))
    story.append(prec_table)

    story.append(PageBreak())

    # =========================================================================
    # BÖLÜM 5: KONSOL ÇIKTISI (say DEYİMİ)
    # =========================================================================
    story.append(Paragraph("5. Konsol Çıktısı: 'say' Deyimi", styles["SectionHeading"]))
    story.append(HRFlowable(width="100%", thickness=1.5, color=COLOR_PRIMARY, spaceAfter=10))

    story.append(Paragraph(
        "MRT 0.2.0'da konsola metin ve değer yazdırmak için <code>say</code> deyimi kullanılır. "
        "<code>say</code> bir fonksiyon çağrısı (print(...)) değil, dilin çekirdek sözdizimine yerleşik <b>birinci sınıf bir deyimdir (statement)</b>.",
        styles["Body"]
    ))

    story.append(Paragraph("Özellikleri:", styles["SubSectionHeading"]))
    story.append(Paragraph("• Parantez gerektirmez: <code>say &lt;ifade&gt;</code> biçiminde çalışır.", styles["BulletItem"]))
    story.append(Paragraph("• İfadeyi otomatik olarak hesaplar ve ekrana yazdırır.", styles["BulletItem"]))
    story.append(Paragraph("• Çıktının sonuna daima otomatik yeni satır (<code>\\n</code>) ekler.", styles["BulletItem"]))
    story.append(Paragraph("• Dizge olmayan değerleri de doğrudan yazdırabilir.", styles["BulletItem"]))

    story.append(make_code_block("""say "Merhaba!"
say 42
say 3.14 * 2
say yes
say none

// Metin birleştirme ile yazdırma:
var kullanici = "Murat"
say "Hoş geldin, " + kullanici + "!"
""", styles))

    # =========================================================================
    # BÖLÜM 6: KONTROL AKIŞI
    # =========================================================================
    story.append(Paragraph("6. Kontrol Akışı (when, otherwise, repeat)", styles["SectionHeading"]))
    story.append(HRFlowable(width="100%", thickness=1.5, color=COLOR_PRIMARY, spaceAfter=10))

    story.append(Paragraph("Koşullu İfadeler (when / otherwise):", styles["SubSectionHeading"]))
    story.append(Paragraph(
        "MRT'de geleneksel <code>if / else</code> yerine <code>when / otherwise</code> blokları kullanılır. "
        "Bu yapı koda hem okunabilirlik hem de doğal bir dil akışı kazandırır:",
        styles["Body"]
    ))

    story.append(make_code_block("""var notOrtalamasi = 85

when notOrtalamasi >= 90 {
    say "Harf Notu: AA"
} otherwise when notOrtalamasi >= 80 {
    say "Harf Notu: BA"
} otherwise when notOrtalamasi >= 70 {
    say "Harf Notu: BB"
} otherwise {
    say "Kaldınız."
}
""", styles))

    story.append(Paragraph("Döngüler (repeat) ve Akış Kontrolü (break, continue):", styles["SubSectionHeading"]))
    story.append(Paragraph(
        "Yinelenen işlemler için <code>repeat (durum)</code> döngüsü kullanılır. "
        "Döngüyü anında sonlandırmak için <code>break</code>, o adımı atlayıp bir sonraki yinelemeye geçmek için <code>continue</code> deyimleri kullanılır:",
        styles["Body"]
    ))

    story.append(make_code_block("""var sayac = 0
var toplam = 0

repeat sayac < 10 {
    sayac = sayac + 1

    // 5 sayısını toplama dahil etme, atla:
    when sayac == 5 {
        continue
    }

    // 8'e ulaşıldığında döngüyü tamamen sonlandır:
    when sayac == 8 {
        break
    }

    toplam = toplam + sayac
    say "Sayac: " + toText(sayac) + ", Ara Toplam: " + toText(toplam)
}

say "Nihai Toplam: " + toText(toplam)
""", styles))

    story.append(PageBreak())

    # =========================================================================
    # BÖLÜM 7: GÖREVLER (TASKS / FONKSİYONLAR)
    # =========================================================================
    story.append(Paragraph("7. Görevler (Tasks / Fonksiyonlar)", styles["SectionHeading"]))
    story.append(HRFlowable(width="100%", thickness=1.5, color=COLOR_PRIMARY, spaceAfter=10))

    story.append(Paragraph(
        "MRT'de fonksiyonlar <code>task</code> anahtar sözcüğü ile tanımlanır. "
        "Bir görevden değer döndürmek için <code>give</code> deyimi kullanılır. "
        "Eğer bir görevde <code>give</code> çağrılmazsa, görev varsayılan olarak <code>none</code> değeri üretir.",
        styles["Body"]
    ))

    story.append(make_code_block("""// Temel Görev Tanımı
task topla(a, b) {
    give a + b
}

var sonuc = topla(15, 25)
say "Sonuç: " + toText(sonuc) // 40
""", styles))

    story.append(Paragraph("Sözcüksel Kapsam ve Değişken Gölgeleme (Scoping & Shadowing):", styles["SubSectionHeading"]))
    story.append(Paragraph(
        "Her görev ve kod bloğu (<code>{ ... }</code>) yeni bir sözcüksel kapsam (lexical scope) oluşturur. "
        "İç blokta tanımlanan bir değişken dış bloktaki değişkeni geçici olarak gölgeler, dış değişkenin değerini bozmaz:",
        styles["Body"]
    ))

    story.append(make_code_block("""var x = 100

task ornek() {
    var x = 50 // Dıştaki x'i gölgeler
    say "Görev içi x: " + toText(x) // 50
}

ornek()
say "Görev dışı x: " + toText(x) // 100 (değişmedi)
""", styles))

    story.append(Paragraph("Özyineleme (Recursion):", styles["SubSectionHeading"]))
    story.append(Paragraph("MRT, özyinelemeli fonksiyon çağrılarını eksiksiz destekler:", styles["Body"]))

    story.append(make_code_block("""// Faktöriyel Hesabı
task faktoriyel(n) {
    when n <= 1 {
        give 1
    }
    give n * faktoriyel(n - 1)
}

say "5! = " + toText(faktoriyel(5))   // 120

// Fibonacci Hesabı
task fib(n) {
    when n <= 1 {
        give n
    }
    give fib(n - 1) + fib(n - 2)
}

say "fib(10) = " + toText(fib(10))   // 55
""", styles))

    # =========================================================================
    # BÖLÜM 8: YERLEŞİK STANDART KÜTÜPHANE
    # =========================================================================
    story.append(Paragraph("8. Yerleşik Standart Kütüphane", styles["SectionHeading"]))
    story.append(HRFlowable(width="100%", thickness=1.5, color=COLOR_PRIMARY, spaceAfter=10))

    story.append(Paragraph("MRT çekirdeğinde kullanıma hazır temel yerleşik işlevler yer alır:", styles["Body"]))

    builtins_data = [
        [Paragraph("Fonksiyon", styles["TableHead"]), Paragraph("Dönüş Tipi", styles["TableHead"]), Paragraph("Açıklama ve Kullanım Örneği", styles["TableHead"])],
        [
            Paragraph("read([mesaj])", styles["TableCellCode"]),
            Paragraph("string / none", styles["TableCell"]),
            Paragraph("Standart girdiden (stdin) bir satır okur. İsteğe bağlı olarak ekrana istem mesajı basar. Dosya sonu (EOF) durumunda <code>none</code> döner.", styles["TableCell"])
        ],
        [
            Paragraph("toNumber(değer)", styles["TableCellCode"]),
            Paragraph("integer / float", styles["TableCell"]),
            Paragraph("Metin, tam sayı, ondalık veya mantıksal değeri sayıya dönüştürür. Geçersiz metinlerde tip hatası üretir.", styles["TableCell"])
        ],
        [
            Paragraph("toText(değer)", styles["TableCellCode"]),
            Paragraph("string", styles["TableCell"]),
            Paragraph("Herhangi bir değeri metne dönüştürür. Sayılarla dizgeleri birleştirirken kullanılır.", styles["TableCell"])
        ],
        [
            Paragraph("typeOf(değer)", styles["TableCellCode"]),
            Paragraph("string", styles["TableCell"]),
            Paragraph("Verilen değerin tip adını metin olarak döner: <code>\"integer\"</code>, <code>\"float\"</code>, <code>\"string\"</code>, <code>\"boolean\"</code>, <code>\"none\"</code>, <code>\"function\"</code>.", styles["TableCell"])
        ],
        [
            Paragraph("length(metin)", styles["TableCellCode"]),
            Paragraph("integer", styles["TableCell"]),
            Paragraph("Verilen dizgenin karakter sayısını döner: <code>length(\"MRT\") -> 3</code>.", styles["TableCell"])
        ],
        [
            Paragraph("clock()", styles["TableCellCode"]),
            Paragraph("float", styles["TableCell"]),
            Paragraph("Programın başladığı andan itibaren geçen CPU süresini saniye cinsinden döner.", styles["TableCell"])
        ],
    ]
    builtins_table = Table(builtins_data, colWidths=[36 * mm, 24 * mm, 110 * mm])
    builtins_table.setStyle(TableStyle([
        ('BACKGROUND', (0, 0), (-1, 0), COLOR_PRIMARY),
        ('BOX', (0, 0), (-1, -1), 1, COLOR_BORDER),
        ('INNERGRID', (0, 0), (-1, -1), 0.5, COLOR_BORDER),
        ('ROWBACKGROUNDS', (0, 1), (-1, -1), [colors.white, COLOR_BG_LIGHT]),
        ('TOPPADDING', (0, 0), (-1, -1), 5),
        ('BOTTOMPADDING', (0, 0), (-1, -1), 5),
        ('LEFTPADDING', (0, 0), (-1, -1), 8),
        ('RIGHTPADDING', (0, 0), (-1, -1), 8),
    ]))
    story.append(builtins_table)

    story.append(Spacer(1, 3 * mm))
    story.append(Paragraph("Kullanıcıdan Girdi Alma Örneği:", styles["SubSectionHeading"]))
    story.append(make_code_block("""// Kullanıcı etkileşimi: read() ve toNumber()
var isim = read("Adınız: ")
say "Merhaba, " + isim + "!"

var yas = toNumber(read("Yaşınız: "))
say "Gelecek yıl yaşınız: " + toText(yas + 1)
""", styles))

    story.append(PageBreak())

    # =========================================================================
    # BÖLÜM 9: HATA YÖNETİMİ VE TANILAMA
    # =========================================================================
    story.append(Paragraph("9. Hata Yönetimi ve Tanılama", styles["SectionHeading"]))
    story.append(HRFlowable(width="100%", thickness=1.5, color=COLOR_PRIMARY, spaceAfter=10))

    story.append(Paragraph(
        "MRT, modern derleyicilerdekine (GCC, Clang, Rust) benzer nitelikte gelişmiş hata teşhis mekanizmasına sahiptir. "
        "Bir hata meydana geldiğinde yalnızca mesaj basılmaz; kaynak dosyanın ilgili satırı, sütunu ve hatanın tam konumu "
        "imleç (<code>^</code>) işaretiyle görsel olarak gösterilir.",
        styles["Body"]
    ))

    story.append(Paragraph("Hata Kategorileri:", styles["SubSectionHeading"]))
    story.append(Paragraph("• <b>SyntaxError:</b> Sözdizimi kuralı ihlalleri (eksik parantez, geçersiz token, vb.).", styles["BulletItem"]))
    story.append(Paragraph("• <b>NameError:</b> Tanımlanmamış bir değişkene veya göreve erişim denemesi.", styles["BulletItem"]))
    story.append(Paragraph("• <b>TypeError:</b> Uyumsuz tipler arası işlem (örneğin bir tamsayı ile mantıksal değeri toplama).", styles["BulletItem"]))
    story.append(Paragraph("• <b>RuntimeError:</b> Çalışma zamanında sıfıra bölme gibi aritmetik veya mantıksal hatalar.", styles["BulletItem"]))

    story.append(Paragraph("Örnek Görsel Hata Çıktısı:", styles["SubSectionHeading"]))
    story.append(make_code_block("""ornek.mrt:4:12

    say 100 / 0
               ^

RuntimeError: division by zero
""", styles))

    # =========================================================================
    # BÖLÜM 10: MRT STUDIO IDE KILAVUZU
    # =========================================================================
    story.append(Paragraph("10. MRT Studio IDE Kılavuzu", styles["SectionHeading"]))
    story.append(HRFlowable(width="100%", thickness=1.5, color=COLOR_PRIMARY, spaceAfter=10))

    story.append(Paragraph(
        "<b>MRT Studio</b>, MRT geliştiricileri için sıfırdan <b>GTK4</b> ve <b>GtkSourceView 5</b> kullanılarak C diliyle yazılmış yerel Linux geliştirme ortamıdır. "
        "Web teknolojileri veya ağır runtime'lar barındırmaz; açılışı anlıktır ve bellek tüketimi düşüktür.",
        styles["Body"]
    ))

    story.append(Paragraph("Öne Çıkan IDE Yetenekleri:", styles["SubSectionHeading"]))

    ide_features = [
        ("Çift Dilli Arayüz (TR / EN)", "Sistem diline otomatik uyum sağlar. Ayarlar penceresinden anında Türkçe veya İngilizce seçilebilir. Yedek sözlük mimarisi sayesinde sistemde tr_TR yereli olmasa da çalışır."),
        ("Komut Paleti (Ctrl+Shift+P)", "Tüm IDE komutlarını klavyeden arayıp tek tuşla tetikleyebileceğiniz modern komut çubuğu."),
        ("Hızlı Dosya Aç (Ctrl+P)", "Projedeki tüm kaynak dosyaları anında filtreleyip doğrudan düzenleme sekmesine getiren araç."),
        ("Dosyalarda Bul (Ctrl+Shift+F)", "Proje genelindeki tüm .mrt dosyalarında metin araması yapar; eşleşen satıra ve sütuna tıklandığında imleci doğrudan hata/kod satırına götürür."),
        ("Satıra Git (Ctrl+G)", "Büyük dosyalarda istenen satır numarasına doğrudan atlama kutusu."),
        ("Sembol Ağacı (Outline)", "Sol kenar çubuğunda açılan dosyada tanımlı tüm 'task' ve 'var' sembollerini gerçek zamanlı listeler; tıklandığında tanımlandığı satıra odaklanır."),
        ("Sorunlar & Çıktı Paneli", "MRT programlarının stdout/stderr akışını asenkron gösterir. Hata satırları parse edilir ve Sorunlar sekmesinde tıklanabilir hata listesi sunulur."),
        ("Otomatik Yedekleme & Kurtarma", "15 saniyede bir değiştirilen dosyaları ~/.local/state/mrt-studio/recovery/ dizininde güvene alır. Olası çökme veya elektrik kesintisinde açılışta dosyaları geri yükler."),
        ("Yeni Proje Sihirbazı", "Konsol Uygulaması (Console), Boş Proje (Empty) ve Kütüphane (Library) şablonları ile saniyeler içinde yeni proje iskeleti kurar."),
    ]

    for f_title, f_desc in ide_features:
        story.append(Paragraph(f"• <b>{f_title}:</b> {f_desc}", styles["BulletItem"]))

    story.append(Spacer(1, 3 * mm))
    story.append(Paragraph("Klavye Kısayolları Tablosu:", styles["SubSectionHeading"]))

    shortcuts_data = [
        [Paragraph("Kısayol", styles["TableHead"]), Paragraph("Eylem", styles["TableHead"]), Paragraph("Kısayol", styles["TableHead"]), Paragraph("Eylem", styles["TableHead"])],
        [Paragraph("Ctrl+N", styles["TableCellCode"]), Paragraph("Yeni Dosya", styles["TableCell"]), Paragraph("F5", styles["TableCellCode"]), Paragraph("Programı Çalıştır (Run)", styles["TableCell"])],
        [Paragraph("Ctrl+O", styles["TableCellCode"]), Paragraph("Dosya Aç", styles["TableCell"]), Paragraph("Ctrl+Shift+P", styles["TableCellCode"]), Paragraph("Komut Paleti", styles["TableCell"])],
        [Paragraph("Ctrl+S", styles["TableCellCode"]), Paragraph("Kaydet", styles["TableCell"]), Paragraph("Ctrl+P", styles["TableCellCode"]), Paragraph("Hızlı Dosya Aç", styles["TableCell"])],
        [Paragraph("Ctrl+Shift+S", styles["TableCellCode"]), Paragraph("Farklı Kaydet", styles["TableCell"]), Paragraph("Ctrl+Shift+F", styles["TableCellCode"]), Paragraph("Dosyalarda Bul", styles["TableCell"])],
        [Paragraph("Ctrl+Alt+S", styles["TableCellCode"]), Paragraph("Tümünü Kaydet", styles["TableCell"]), Paragraph("Ctrl+G", styles["TableCellCode"]), Paragraph("Satıra Git", styles["TableCell"])],
        [Paragraph("Ctrl+F", styles["TableCellCode"]), Paragraph("Bul", styles["TableCell"]), Paragraph("Ctrl+B", styles["TableCellCode"]), Paragraph("Kenar Çubuğunu Gizle/Aç", styles["TableCell"])],
        [Paragraph("Ctrl+H", styles["TableCellCode"]), Paragraph("Bul & Değiştir", styles["TableCell"]), Paragraph("Ctrl+J", styles["TableCellCode"]), Paragraph("Çıktı Panelini Gizle/Aç", styles["TableCell"])],
        [Paragraph("Ctrl+Z", styles["TableCellCode"]), Paragraph("Geri Al (Undo)", styles["TableCell"]), Paragraph("Ctrl+Q", styles["TableCellCode"]), Paragraph("MRT Studio'dan Çıkış", styles["TableCell"])],
    ]
    shortcuts_table = Table(shortcuts_data, colWidths=[26 * mm, 59 * mm, 30 * mm, 55 * mm])
    shortcuts_table.setStyle(TableStyle([
        ('BACKGROUND', (0, 0), (-1, 0), COLOR_SECONDARY),
        ('BOX', (0, 0), (-1, -1), 1, COLOR_BORDER),
        ('INNERGRID', (0, 0), (-1, -1), 0.5, COLOR_BORDER),
        ('ROWBACKGROUNDS', (0, 1), (-1, -1), [colors.white, COLOR_BG_LIGHT]),
        ('TOPPADDING', (0, 0), (-1, -1), 4),
        ('BOTTOMPADDING', (0, 0), (-1, -1), 4),
        ('LEFTPADDING', (0, 0), (-1, -1), 6),
        ('RIGHTPADDING', (0, 0), (-1, -1), 6),
    ]))
    story.append(shortcuts_table)

    story.append(PageBreak())

    # =========================================================================
    # BÖLÜM 11: MİMARİ VE GELECEK YOL HARİTASI
    # =========================================================================
    story.append(Paragraph("11. Dil Mimarisi ve Gelecek Yol Haritası", styles["SectionHeading"]))
    story.append(HRFlowable(width="100%", thickness=1.5, color=COLOR_PRIMARY, spaceAfter=10))

    story.append(Paragraph(
        "MRT dil motoru, derleyici tasarımının temel ilkelerine tam sadakatle inşa edilmiştir. "
        "Mimari, temiz bir modüler ayrım ve genişletilebilirlik üzerine kuruludur:",
        styles["Body"]
    ))

    story.append(make_code_block("""MRT Kaynak Kodu (.mrt)
         ↓
    [ Lexer ]       --> Karakter akışını tarar, Token üretir (src/lexer.c)
         ↓
    [ Parser ]      --> Recursive Descent + Precedence Climbing (src/parser.c)
         ↓
       AST          --> Soyut Sözdizim Ağacı (src/ast.c) [ÖN YÜZ SINIRI]
         ↓
  [ Interpreter ]   --> AST Tree-Walking Çalıştırma Motoru (src/interpreter.c)
    ├── Environment --> Sözcüksel Kapsam Zinciri (src/environment.c)
    ├── Value       --> Referans Sayımlı Tagged Union (src/value.c)
    └── Builtins    --> typeOf, length, toText, read, toNumber, clock (src/builtin.c)
""", styles))

    story.append(Paragraph("Gelecek Yol Haritası (Roadmap):", styles["SubSectionHeading"]))
    story.append(Paragraph("• <b>MRT 0.3.0 — Bytecode VM:</b> AST'den doğrudan bytecode derlemesi ve yığın tabanlı (stack-based) hızlı bir sanal makine geliştirilmesi.", styles["BulletItem"]))
    story.append(Paragraph("• <b>MRT 0.4.0 — C Transpiler:</b> MRT kodlarını optimize edilmiş C koduna dönüştürerek GCC/Clang ile doğrudan yerel ikili (native binary) üretme kabiliyeti.", styles["BulletItem"]))
    story.append(Paragraph("• <b>MRT Paket Yöneticisi:</b> Kütüphaneleri ve modülleri kolayca yükleyip yönetecek CLI aracı.", styles["BulletItem"]))
    story.append(Paragraph("• <b>Standart Kütüphane Genişlemesi:</b> Dosya I/O, matematik ve ağ soket desteği.", styles["BulletItem"]))

    # =========================================================================
    # BÖLÜM 12: HIZLI BAŞVURU KARTI (CHEAT SHEET)
    # =========================================================================
    story.append(Spacer(1, 4 * mm))
    story.append(Paragraph("12. Hızlı Başvuru Kartı (Cheat Sheet)", styles["SectionHeading"]))
    story.append(HRFlowable(width="100%", thickness=1.5, color=COLOR_PRIMARY, spaceAfter=10))

    cheatsheet_code = """// MRT 0.2.0 Tam Özet Kod Bloğu

// 1. Değişkenler ve Tipler
var tamsayi = 42
var ondalik = 3.14
var metin = "MRT Dili"
var aktif = yes
var pasif = no
var bos = none

// 2. Konsola Yazdırma ve Kullanıcı Girdisi
say "Sayı: " + toText(tamsayi)
var isim = read("Adınız: ")
var yas = toNumber(read("Yaş: "))

// 3. Koşul Yapısı
when tamsayi > 50 {
    say "Büyük"
} otherwise when tamsayi == 42 {
    say "Doğru Cevap!"
} otherwise {
    say "Küçük"
}

// 4. Döngü ve Akış Kontrolü
var i = 0
repeat i < 5 {
    i = i + 1
    when i == 2 { continue }
    when i == 4 { break }
    say "i = " + toText(i)
}

// 5. Görev Tanımı ve Çağrısı
task topla(a, b) {
    give a + b
}

say "10 + 20 = " + toText(topla(10, 20))
"""
    story.append(make_code_block(cheatsheet_code, styles))

    doc.build(story, canvasmaker=NumberedCanvas)
    print(f"[OK] PDF başarıyla üretildi: {filename}")


if __name__ == "__main__":
    out_pdf = "docs/MRT_Programlama_Dili_El_Kitabi.pdf"
    if len(sys.argv) > 1:
        out_pdf = sys.argv[1]
    build_pdf(out_pdf)
