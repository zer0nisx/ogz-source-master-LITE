# Código XML para Agregar a Shop.xml

## Ubicación

Agregar el siguiente código XML en el archivo:
**`build/win32/bin/Release/Interface/default/Shop.xml`**

Después de la línea **368** (después del `</COMBOBOX>` de `Shop_AllEquipmentFilter`)

---

## Código XML a Agregar

```xml
	<!-- Mejora #3: Campo de búsqueda de items -->
	<!-- LABEL : Etiqueta de búsqueda -->
	<LABEL item="Shop_SearchLabel" parent="Shop">
		<FONT>FONTa9</FONT>
		<TEXTCOLOR>
			<R>205</R>
			<G>205</G>
			<B>205</B>
		</TEXTCOLOR>
		<BOUNDS>
			<X>477</X>
			<Y>120</Y>
			<W>100</W>
			<H>20</H>
		</BOUNDS>
		<TEXT>Buscar:</TEXT>
	</LABEL>
	
	<!-- EDIT : Campo de búsqueda -->
	<EDIT item="ShopSearchEdit" parent="Shop">
		<EDITLOOK>DefaultEditLook</EDITLOOK>
		<FONT>FONTa9</FONT>
		<TEXTCOLOR>
			<R>205</R>
			<G>205</G>
			<B>205</B>
		</TEXTCOLOR>
		<BOUNDS>
			<X>549</X>
			<Y>118</Y>
			<W>180</W>
			<H>22</H>
		</BOUNDS>
		<MAXLENGTH>64</MAXLENGTH>
		<VISIBLE>true</VISIBLE>
	</EDIT>
	
	<!-- BUTTON : Botón para mostrar/ocultar búsqueda (opcional, si no existe ShopSearchFrameCaller) -->
	<BMBUTTON item="ShopSearchFrameCaller" parent="Shop">
		<BUTTONLOOK>NormalBmButtonLook</BUTTONLOOK>
		<BOUNDS>
			<X>735</X>
			<Y>118</Y>
			<W>44</W>
			<H>22</H>
		</BOUNDS>
		<STRETCH/>
		<BITMAP type="up">search_off.tga</BITMAP>
		<BITMAP type="over">search_on.tga</BITMAP>
		<BITMAP type="down">search_on.tga</BITMAP>
		<TEXT></TEXT>
	</BMBUTTON>

	<!-- Mejora #4: ComboBox de ordenamiento -->
	<!-- LABEL : Etiqueta de ordenamiento -->
	<LABEL item="Shop_SortLabel" parent="Shop">
		<FONT>FONTa9</FONT>
		<TEXTCOLOR>
			<R>205</R>
			<G>205</G>
			<B>205</B>
		</TEXTCOLOR>
		<BOUNDS>
			<X>477</X>
			<Y>172</Y>
			<W>100</W>
			<H>20</H>
		</BOUNDS>
		<TEXT>Ordenar por:</TEXT>
	</LABEL>
	
	<!-- COMBOBOX : Ordenamiento de items -->
	<COMBOBOX item="ShopSortComboBox" parent="Shop">
		<BUTTONLOOK>ListBoxButtonLook</BUTTONLOOK>
		<LISTBOXLOOK>CustomListBoxLook</LISTBOXLOOK>
		<FONT>FONTa9</FONT>
		<TEXTCOLOR>
			<R>205</R>
			<G>205</G>
			<B>205</B>
		</TEXTCOLOR>
		<TEXTALIGN>
			<HALIGN>center</HALIGN>
			<VALIGN>center</VALIGN>
		</TEXTALIGN>
		<BOUNDS>
			<X>549</X>
			<Y>170</Y>
			<W>230</W>
			<H>25</H>
		</BOUNDS>
		<LISTITEM selected="true">Sin ordenamiento</LISTITEM>
		<LISTITEM>Nombre (A-Z)</LISTITEM>
		<LISTITEM>Nombre (Z-A)</LISTITEM>
		<LISTITEM>Precio (Menor a Mayor)</LISTITEM>
		<LISTITEM>Precio (Mayor a Menor)</LISTITEM>
		<LISTITEM>Nivel (Bajo a Alto)</LISTITEM>
		<LISTITEM>Nivel (Alto a Bajo)</LISTITEM>
		<ITEMHEIGHT>18</ITEMHEIGHT>
		<DROPSIZE>180</DROPSIZE>
	</COMBOBOX>

	<!-- Mejora #2: Checkbox "Puedo Comprar" -->
	<!-- BUTTON : Checkbox para filtrar solo items asequibles -->
	<BUTTON item="ShopFilterAffordable" parent="Shop">
		<BUTTONLOOK>CheckButtonLook</BUTTONLOOK>
		<FONT>FONTa9</FONT>
		<TEXTCOLOR>
			<R>205</R>
			<G>205</G>
			<B>205</B>
		</TEXTCOLOR>
		<BOUNDS>
			<X>477</X>
			<Y>440</Y>
			<W>200</W>
			<H>20</H>
		</BOUNDS>
		<TEXT>Solo items que puedo comprar</TEXT>
		<CHECKABLE>true</CHECKABLE>
		<CHECKED>false</CHECKED>
	</BUTTON>
```

---

## Ubicación Visual en el Layout

```
┌─────────────────────────────────────────────────┐
│  Shop Interface                                 │
├─────────────────────────────────────────────────┤
│  [Buscar:        ] [Campo de búsqueda] [🔍]    │  ← Nueva línea (Y:118-120)
│                                                 │
│  [장비 종류 :] [ComboBox de filtro tipo]       │  ← Existente (Y:145)
│                                                 │
│  [Ordenar por:] [ComboBox de ordenamiento]     │  ← Nueva línea (Y:170-172)
│                                                 │
│  ┌─────────────────────────────────────────┐   │
│  │ Lista de Items                          │   │
│  │                                         │   │
│  │                                         │   │
│  └─────────────────────────────────────────┘   │
│                                                 │
│  [☐ Solo items que puedo comprar]              │  ← Nueva línea (Y:440)
│                                                 │
│  [Botón Comprar/Vender]                        │  ← Existente (Y:449)
└─────────────────────────────────────────────────┘
```

---

## Notas Importantes

### 1. Coordenadas
- **Búsqueda**: Y=118-120 (justo arriba del filtro de tipo)
- **Ordenamiento**: Y=170-172 (justo debajo del filtro de tipo)
- **Checkbox "Puedo Comprar"**: Y=440 (justo arriba del botón de compra)

### 2. Ajustes de Posición
Si hay conflictos de posición, ajusta las coordenadas Y:
- Búsqueda: Puede ir en Y=118 o Y=95 (más arriba)
- Ordenamiento: Puede ir en Y=172 o Y=200 (más abajo)
- Checkbox: Puede ir en Y=440 o Y=420 (más arriba)

### 3. Textos
Los textos están en español. Si necesitas traducirlos:
- "Buscar:" → "Search:" (inglés) o usar STR:UI_SHOP_SEARCH
- "Ordenar por:" → "Sort by:" (inglés) o usar STR:UI_SHOP_SORT
- "Solo items que puedo comprar" → "Only affordable items" (inglés) o usar STR:UI_SHOP_FILTER_AFFORDABLE

### 4. Bitmaps
- `search_off.tga` y `search_on.tga` - Si no existen, puedes usar otros bitmaps existentes o crear nuevos
- O simplemente ocultar el botón si no es necesario

### 5. Look Styles
- `DefaultEditLook` - Para el campo de texto
- `CheckButtonLook` - Para el checkbox (si existe, sino usar `DefaultButtonLook`)

---

## Versión con Textos usando STR (Recomendado)

Si prefieres usar el sistema de strings del juego:

```xml
	<!-- Mejora #3: Campo de búsqueda de items -->
	<LABEL item="Shop_SearchLabel" parent="Shop">
		<FONT>FONTa9</FONT>
		<TEXTCOLOR>
			<R>205</R>
			<G>205</G>
			<B>205</B>
		</TEXTCOLOR>
		<BOUNDS>
			<X>477</X>
			<Y>120</Y>
			<W>100</W>
			<H>20</H>
		</BOUNDS>
		<TEXT>STR:UI_SHOP_SEARCH_LABEL</TEXT>
	</LABEL>
	
	<EDIT item="ShopSearchEdit" parent="Shop">
		<EDITLOOK>DefaultEditLook</EDITLOOK>
		<FONT>FONTa9</FONT>
		<TEXTCOLOR>
			<R>205</R>
			<G>205</G>
			<B>205</B>
		</TEXTCOLOR>
		<BOUNDS>
			<X>549</X>
			<Y>118</Y>
			<W>180</W>
			<H>22</H>
		</BOUNDS>
		<MAXLENGTH>64</MAXLENGTH>
		<VISIBLE>true</VISIBLE>
	</EDIT>

	<!-- Mejora #4: ComboBox de ordenamiento -->
	<LABEL item="Shop_SortLabel" parent="Shop">
		<FONT>FONTa9</FONT>
		<TEXTCOLOR>
			<R>205</R>
			<G>205</G>
			<B>205</B>
		</TEXTCOLOR>
		<BOUNDS>
			<X>477</X>
			<Y>172</Y>
			<W>100</W>
			<H>20</H>
		</BOUNDS>
		<TEXT>STR:UI_SHOP_SORT_LABEL</TEXT>
	</LABEL>
	
	<COMBOBOX item="ShopSortComboBox" parent="Shop">
		<BUTTONLOOK>ListBoxButtonLook</BUTTONLOOK>
		<LISTBOXLOOK>CustomListBoxLook</LISTBOXLOOK>
		<FONT>FONTa9</FONT>
		<TEXTCOLOR>
			<R>205</R>
			<G>205</G>
			<B>205</B>
		</TEXTCOLOR>
		<TEXTALIGN>
			<HALIGN>center</HALIGN>
			<VALIGN>center</VALIGN>
		</TEXTALIGN>
		<BOUNDS>
			<X>549</X>
			<Y>170</Y>
			<W>230</W>
			<H>25</H>
		</BOUNDS>
		<LISTITEM selected="true">STR:UI_SHOP_SORT_NONE</LISTITEM>
		<LISTITEM>STR:UI_SHOP_SORT_NAME_ASC</LISTITEM>
		<LISTITEM>STR:UI_SHOP_SORT_NAME_DESC</LISTITEM>
		<LISTITEM>STR:UI_SHOP_SORT_PRICE_ASC</LISTITEM>
		<LISTITEM>STR:UI_SHOP_SORT_PRICE_DESC</LISTITEM>
		<LISTITEM>STR:UI_SHOP_SORT_LEVEL_ASC</LISTITEM>
		<LISTITEM>STR:UI_SHOP_SORT_LEVEL_DESC</LISTITEM>
		<ITEMHEIGHT>18</ITEMHEIGHT>
		<DROPSIZE>180</DROPSIZE>
	</COMBOBOX>

	<!-- Mejora #2: Checkbox "Puedo Comprar" -->
	<BUTTON item="ShopFilterAffordable" parent="Shop">
		<BUTTONLOOK>CheckButtonLook</BUTTONLOOK>
		<FONT>FONTa9</FONT>
		<TEXTCOLOR>
			<R>205</R>
			<G>205</G>
			<B>205</B>
		</TEXTCOLOR>
		<BOUNDS>
			<X>477</X>
			<Y>440</Y>
			<W>200</W>
			<H>20</H>
		</BOUNDS>
		<TEXT>STR:UI_SHOP_FILTER_AFFORDABLE</TEXT>
		<CHECKABLE>true</CHECKABLE>
		<CHECKED>false</CHECKED>
	</BUTTON>
```

Luego agregar los strings en el archivo de mensajes correspondiente.

---

## Instrucciones de Instalación

1. Abrir el archivo: `build/win32/bin/Release/Interface/default/Shop.xml`
2. Buscar la línea 368 (después de `</COMBOBOX>` de `Shop_AllEquipmentFilter`)
3. Copiar y pegar el código XML proporcionado
4. Ajustar coordenadas si es necesario para evitar solapamientos
5. Guardar el archivo
6. Probar en el juego

---

## Verificación

Después de agregar el código, verificar que:
- ✅ Los widgets no se solapen con otros elementos
- ✅ Los textos sean visibles
- ✅ Los widgets respondan a clicks/interacciones
- ✅ El campo de búsqueda acepte texto
- ✅ El ComboBox muestre las opciones correctamente
- ✅ El checkbox cambie de estado al hacer click

---

**Nota**: Si algún widget no aparece o no funciona, verificar:
1. Que las coordenadas estén dentro de los límites del frame padre
2. Que los nombres de los widgets coincidan exactamente con los del código C++
3. Que los "LOOK" styles existan (si no, usar estilos por defecto)

